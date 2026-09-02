#include "vertex_raw_data.hpp"
#include "vk_buffers.hpp"
#include "vk_engine.hpp"
#include "vk_types.hpp"
#include "vk_util.hpp"
#include "vulkan/vulkan.hpp"
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
VkEngine::RenderAttachments VkEngine::prepare_render_attachments(
    vk::raii::CommandBuffer const& cmdBuf,
    u32 imageIndex,
    std::array<f32, 4> clearColor
){
    auto const& swapImage =  m_swapchain.images.at(imageIndex);
    vk_util::transition_image_layout(
        cmdBuf, swapImage, 
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,  
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor
    );
    vk_util::transition_image_layout(
        cmdBuf, m_vkDepthImage, 
        vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::ImageAspectFlagBits::eDepth
    );

    auto clearColorVal = vk::ClearValue{}.setColor(
            vk::ClearColorValue{} .setFloat32(
                clearColor
            )
        )
    ;
    vk::ClearValue clearDepthVal = vk::ClearValue{}
        .setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0))
    ;

    auto attachmentInfo = vk::RenderingAttachmentInfo {}
        .setImageView(m_swapchain.imageViews.at(imageIndex))
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(clearColorVal)
    ;
    auto depthAttachmentInfo = vk::RenderingAttachmentInfo{}
        .setImageView(m_vkDepthImageView)
        .setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setClearValue(clearDepthVal)
    ;
    ASSERT(attachmentInfo.sType == vk::StructureType::eRenderingAttachmentInfo);

    return {attachmentInfo, depthAttachmentInfo};
}
// aka recordCommadBuffer in tutorial
void VkEngine::draw_mesh(vk::raii::CommandBuffer const& cmdBuf, GpuMesh const& gpu_mesh){
    cmdBuf.bindVertexBuffers(0, gpu_mesh.vertices.buffer, {0});
    cmdBuf.bindIndexBuffer(gpu_mesh.indices.buffer, 0, m_gpu_heightmapMesh.index_type);
    set_dynamic_state(cmdBuf);

    cmdBuf.drawIndexed(gpu_mesh.m_index_count, 1, 0,0,0);
}
void VkEngine::record_commands(vk::raii::CommandBuffer const& cmdBuf, u32 imageIndex){

    auto const frameIndex = get_current_frame_index();
    auto const& swapchainImage = m_swapchain.images.at(imageIndex);

    cmdBuf.reset();
    cmdBuf.begin({});
    cmdBuf.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        m_vkPipelineLayout,
        0, 
        *m_vkDescriptorSets[frameIndex],
        nullptr
    );


    std::array<float, 4> clearColor = {
        std::abs(std::sin(m_frameCount / 120.0f)),
        0.0f,
        std::abs(std::sin(m_frameCount / 120.0f)),
        1.0f
    };
    auto renderAttachments = prepare_render_attachments(cmdBuf, imageIndex, clearColor);
    cmdBuf.beginRendering(renderAttachments.get_info(m_swapchain));
    cmdBuf.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        *m_vkPipeline
    );
    draw_mesh(cmdBuf, m_gpu_heightmapMesh);

    cmdBuf.endRendering();
    vk_util::transition_image_layout(
        cmdBuf, swapchainImage, 
        vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {}, // must be eNone for ePresentSrcKHR
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,/* src stage mask*/
        vk::PipelineStageFlagBits2::eBottomOfPipe,
        vk::ImageAspectFlagBits::eColor
    );

    cmdBuf.end();
}

void VkEngine::update_uniforms(FrameData const& frame) {
    auto aspect = m_swapchain.extent.width / static_cast<f32>( m_swapchain.extent.height);
    auto model = glm::mat4(1.0f);
    model = glm::translate(model,m_heightmap.m_world_center);
//    model = glm::scale(model,glm::vec3{10.0f,10.0f, 1.0f});
    auto ubo = UniformBufferObject{
        .model = model,
        .view = m_cam.get_view_matrix(),
        .proj = m_cam.get_proj_matrix(aspect),
    };
    // HACK: glm uses y up for clip space, vulkan uses y down. flip here
    ubo.proj[1][1] *= -1; 
    memcpy(frame.uniformBufferMappedMemory, &ubo,sizeof(ubo));
}
void VkEngine::present_image(u32 imageIndex, vk::Result acquire_res){
    auto present_rv = m_vkQueue.presentKHR(
        vk::PresentInfoKHR{}
            .setWaitSemaphores(*m_swapchain.renderFinishedSemaphores[imageIndex])
            .setSwapchains(*m_swapchain.descriptor)
            .setImageIndices(imageIndex)
    );
    if (   present_rv == vk::Result::eSuboptimalKHR 
        || present_rv == vk::Result::eErrorOutOfDateKHR
        || acquire_res == vk::Result::eSuboptimalKHR // since we ignore this in the previous check
        || m_framebufferResized
    ){
        m_framebufferResized = false;
        recreate_swapchain();
    }else{
        ASSERT(present_rv==vk::Result::eSuccess);
    }
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
    m_vkDevice.resetFences(*frame.fence);
    update_uniforms(frame);

    record_commands(frame.commandBuffer, imageIndex);

    auto waitDstStageMask = static_cast<vk::PipelineStageFlags>(
        vk::PipelineStageFlagBits::eColorAttachmentOutput
    );

    m_vkQueue.submit(
        vk::SubmitInfo{}
            .setCommandBuffers(*frame.commandBuffer)
            .setWaitSemaphores(*frame.presentCompleteSemaphore)
            .setSignalSemaphores(*m_swapchain.renderFinishedSemaphores[imageIndex])
            .setWaitDstStageMask(waitDstStageMask),
        *frame.fence
    );

    present_image(imageIndex, acquire_res);

    m_frameCount++;
}


