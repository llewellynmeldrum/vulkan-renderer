#include "vertex_raw_data.hpp"
#include "vk_buffers.hpp"
#include "vk_engine.hpp"
#include "vk_types.hpp"
#include "vk_util.hpp"
#include <ranges>
#include <thread>

void VkEngine::set_dynamic_state(vk::raii::CommandBuffer const& cmdBuf){
    // we specified viewport and scissor rect to be dynamic, thus we must set them before the draw cmd
    static_assert(std::ranges::contains(vk_enabledDynamicState, vk::DynamicState::eViewport));
    cmdBuf.setViewport(
        0, 
        dyn_get_viewport()
    );

    static_assert(std::ranges::contains(vk_enabledDynamicState, vk::DynamicState::eScissor));
    cmdBuf.setScissor(
        0,
        dyn_get_scissor()
    );

    static_assert(std::ranges::contains(vk_enabledDynamicState, vk::DynamicState::ePolygonModeEXT));
    cmdBuf.setPolygonModeEXT(
        dyn_get_polymode()
    );
}
// aka recordCommadBuffer in tutorial
void VkEngine::record_commands_to_buffer(u32 imageIndex){
    auto& cmdBuf = get_current_frame().commandBuffer;
    auto const frame_idx = get_current_frame_index();
    cmdBuf.begin({});
    cmdBuf.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        m_vkPipelineLayout,
        0, 
        *m_vkDescriptorSets[frame_idx],
        nullptr
    );
    auto const swapchainImage = m_swapchain.images.at(imageIndex);

    vk_util::transition_image_layout(
        cmdBuf, swapchainImage, 
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,  
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );

    auto clearColor = vk::ClearColorValue{.float32 = {{
        std::abs(std::sin(m_frameCount / 120.0f)),
        0.0f,
        std::abs(std::sin(m_frameCount / 120.0f)),
        1.0f
        }}
    };

    auto attachmentInfo = vk::RenderingAttachmentInfo {
        .imageView = m_swapchain.imageViews.at(imageIndex),
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = {clearColor},
    };
    auto renderingInfo = vk::RenderingInfo{
        .renderArea{.offset={},.extent=m_swapchain.extent},
        .layerCount = 1,
    };
    renderingInfo.setColorAttachments(attachmentInfo);

    cmdBuf.beginRendering(renderingInfo);
    cmdBuf.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        *m_vkPipeline
    );
    cmdBuf.bindVertexBuffers(0, *m_vertexBuffer, {0});
    cmdBuf.bindIndexBuffer(*m_indexBuffer, 0, vtx_raw_data::vk_IndexType(vtx_raw_data::ccw_quad_indices));
    set_dynamic_state(cmdBuf);

    cmdBuf.drawIndexed(vtx_raw_data::idx_count(vtx_raw_data::ccw_quad_indices),1, 0,0,0);

    cmdBuf.endRendering();
    vk_util::transition_image_layout(
        cmdBuf, swapchainImage, 
        vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {}, // must be eNone for ePresentSrcKHR
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,/* src stage mask*/
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );

    cmdBuf.end();
}

void VkEngine::update_uniforms(FrameData const& frame) {
    auto aspect = m_swapchain.extent.width / static_cast<f32>( m_swapchain.extent.height);
    auto model = glm::mat4(1.0f);
    model = glm::translate(model,glm::vec3{2.0f,2.0f, 4.0f});
    model = glm::scale(model,glm::vec3{10.0f,10.0f, 1.0f});
    auto ubo = UniformBufferObject{
        .model = model,
        .view = m_cam.get_view_matrix(),
        .proj = m_cam.get_proj_matrix(aspect),
    };
    // HACK: glm uses y up for clip space, vulkan uses y down. flip here
    ubo.proj[1][1] *= -1; 
    memcpy(frame.uniformBufferMappedMemory, &ubo,sizeof(ubo));

}
void VkEngine::draw() {
    // We perform this early check here in order to prevent a resized framebuffer to present a previous image
    if (m_framebufferResized){
        m_framebufferResized = false;
        recreate_swapchain();
    }
    auto& frame = get_current_frame();
    // 1. wait for previous frame to signal the fence
    auto fenceRV = m_vkDevice.waitForFences(*frame.fence, vk::True, numeric_max<u64>);
    if (fenceRV != vk::Result::eSuccess){
        LOG_FATAL("Failed to wait for fence");
    }

    // 2. Acquire the next image from the swapchain
    auto [acquire_res, imageIndex] = m_swapchain.descriptor.acquireNextImage(
        numeric_max<u64>,
        *frame.presentCompleteSemaphore
    );
    if (acquire_res == vk::Result::eErrorOutOfDateKHR){
        // only early return if we got no image at all, and thus didnt signal the semaphore.
        // Else we get validation error on the semaphore
        recreate_swapchain();
        return;
    }else{
        ASSERT(acquire_res==vk::Result::eSuccess || acquire_res==vk::Result::eSuboptimalKHR);
    }
    // only reset if we acquired an image from the swapchain, else we early returned
    update_uniforms(frame);
    m_vkDevice.resetFences(*frame.fence);

    auto & cmdBuf = frame.commandBuffer;
    cmdBuf.reset();
    record_commands_to_buffer(imageIndex);

    auto waitDstStageMask = static_cast<vk::PipelineStageFlags>(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    m_vkQueue.submit(
        vk::SubmitInfo{}
            .setCommandBuffers(*cmdBuf)
            .setWaitSemaphores(*frame.presentCompleteSemaphore)
            .setSignalSemaphores(*m_swapchain.renderFinishedSemaphores[imageIndex])
            .setWaitDstStageMask(waitDstStageMask),
        *frame.fence
    );

    auto const present_res = m_vkQueue.presentKHR(
        vk::PresentInfoKHR{}
            .setWaitSemaphores(*m_swapchain.renderFinishedSemaphores[imageIndex])
            .setSwapchains(*m_swapchain.descriptor)
            .setImageIndices(imageIndex)
    );
    if (   present_res == vk::Result::eSuboptimalKHR 
        || present_res == vk::Result::eErrorOutOfDateKHR
        || acquire_res == vk::Result::eSuboptimalKHR // since we ignore this in the previous check
        || m_framebufferResized
    ){
        m_framebufferResized = false;
        recreate_swapchain();
    }else{
        ASSERT(present_res==vk::Result::eSuccess);
    }

    m_frameCount++;
}


