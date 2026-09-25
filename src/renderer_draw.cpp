#include <cstddef>
#include <ranges>
#include <thread>

#include "color_utils.hpp"
#include "push_constants.hpp"
#include "vertex_raw_data.hpp"
#include "vk_managed_buffers.hpp"
#include "renderer.hpp"
#include "vk_types.hpp"
#include "vk_util.hpp"

auto upload_ubo(
    void* ubo_mapped_memory,
    UBO ubo
) -> void {
    // HACK: glm uses y up for clip space, vulkan uses y down. flip here
    ubo.proj[1][1] *= -1; 
    memcpy(ubo_mapped_memory, &ubo, sizeof(UBO));
}

auto Renderer::update_ubo(
    FrameData const& frame,
    Camera const& cam
) -> void {
    auto aspect = m_swapchain.extent.width / static_cast<f32>( m_swapchain.extent.height);
    auto ubo = UBO{
        .view = cam.get_view_matrix(),
        .proj = cam.get_proj_matrix(aspect),
    };
    upload_ubo(frame.uniformBufferMappedMemory, ubo);
}


auto Renderer::set_dynamic_state(vk::raii::CommandBuffer const& cmdBuf)
-> void
{
    using std::ranges::contains;
    // we specified viewport and scissor rect to be dynamic, thus we must set them before the draw cmd
    static_assert(contains(vk_enabledDynamicState, vk::DynamicState::eViewport));
    cmdBuf.setViewport(
        0, 
        get_viewport()
    );

    static_assert(contains(vk_enabledDynamicState, vk::DynamicState::eScissor));
    cmdBuf.setScissor(
        0,
        get_scissor()
    );

}
auto Renderer::prepare_render_attachments(
        vk::raii::CommandBuffer const& cmdBuf,
        u32 imageIndex,
        std::array<f32, 4> clearColor
)-> RenderAttachmentContext 
{
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
        cmdBuf, m_depthImage.img.image, 
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
    // NOTE: Normally, we would clear with 1.0f, to represent everything defaulting to far.
    // With flipped Z, far=0.0f, so we need to clear to 0.0f.
    // The reason we flip the Z is so that we keep depth values in the range of a floating point numbers
    // maximum precision, i.e values close to zero. Otherwise, only very far away values get that precision.
    vk::ClearValue clearDepthVal = vk::ClearValue{}
        .setDepthStencil(vk::ClearDepthStencilValue(0.0f, 0))
    ;

    auto attachmentInfo = vk::RenderingAttachmentInfo {}
        .setImageView(m_swapchain.imageViews.at(imageIndex))
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(clearColorVal)
    ;
    auto depthAttachmentInfo = vk::RenderingAttachmentInfo{}
        .setImageView(m_depthImage.img_view)
        .setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setClearValue(clearDepthVal)
    ;
    ASSERT(attachmentInfo.sType == vk::StructureType::eRenderingAttachmentInfo);

    return {attachmentInfo, depthAttachmentInfo};
}
template<typename MeshType>
void draw_mesh(vk::raii::CommandBuffer const& cmdBuf, MeshType const& gpu_mesh){
    ASSERT(gpu_mesh.m_vertex_count!= 0);
    cmdBuf.bindVertexBuffers(0, gpu_mesh.m_vertices.buffer, {0});

    ASSERT(gpu_mesh.m_index_count != 0);
    cmdBuf.bindIndexBuffer(gpu_mesh.m_indices.buffer, 0, gpu_mesh.m_index_type);

    cmdBuf.drawIndexed(gpu_mesh.m_index_count, 1, 0,0,0);
}

void submit_commands(vk::raii::Queue const& q, FrameData const& frame, vk::Semaphore const& signal){
    auto waitDstStageMask = static_cast<vk::PipelineStageFlags>(
        vk::PipelineStageFlagBits::eColorAttachmentOutput
    );

    q.submit(
        vk::SubmitInfo{}
            .setCommandBuffers(*frame.commandBuffer)
            .setWaitSemaphores(*frame.presentCompleteSemaphore)
            .setSignalSemaphores(signal)
            .setWaitDstStageMask(waitDstStageMask),
        *frame.fence
    );
}

auto Renderer::record_commands(
    Camera const& cam,
    FrameData const& frame,
    u32 imageIndex
) -> void {
    auto const& cmdBuf = frame.commandBuffer;
    auto const frameIndex = get_current_frame_index();
    auto const& swapchainImage = m_swapchain.images.at(imageIndex);

    cmdBuf.reset();
    cmdBuf.begin({});


    std::array<float, 4> clearColor = {
        std::abs(std::sin(m_frameCount / 120.0f)),
        0.0f,
        std::abs(std::sin(m_frameCount / 120.0f)),
        1.0f
    };
    set_dynamic_state(cmdBuf);
    auto renderAttachments = prepare_render_attachments(cmdBuf,imageIndex,clearColor);
    cmdBuf.beginRendering(renderAttachments.get_info(m_swapchain));


    // Render 3d 
    auto const& frame_descriptorSets = m_vkDescriptorSets[frameIndex];
    for (const auto& [id, gpu_mesh]: m_gpu_meshes3d){
        LOG_DBG("Drawing mesh id={}, vtx:{},idx:{}",id,gpu_mesh.m_vertex_count, gpu_mesh.m_index_count);
        auto model_matrix = m_model_matrices.at(id);
        // TODO: use push constants here
//        upload_model_matrix(frame.uniformBufferMappedMemory, model_matrix);

        m_fill_pipeline.push(cmdBuf,PushConstants::ModelMatrix{model_matrix});
        m_fill_pipeline.prepare_pass(cmdBuf,frame_descriptorSets);
        draw_mesh(cmdBuf, gpu_mesh);

        m_line_pipeline.push(cmdBuf,PushConstants::ModelMatrix{model_matrix});
        m_line_pipeline.prepare_pass(cmdBuf,frame_descriptorSets);
        draw_mesh(cmdBuf, gpu_mesh);

    }

    auto transform2d = PushConstants::Transform2D{
        // scale should take logical screen pos in and output ndc normalized.
        // shader does: logical_pos * scale + translate
        // To get -1,-1 TL and +1,+1 BR, and 0,0 center, we must :
        // 1. multiply logical_pos by 2, and divide that by window extent.
        //  -> this gives us 0,0 TL, 2,2 BR, 1,1 center.
        .scale = glm::vec2( 2.0 ) / m_windowLogicalExtent,

        // 2. subtract 1 from all of these points and you get -1,-1 TL, 1,1 BR, 0,0 center.
        .translate = glm::vec2(-1.0f)
    };
    m_2d_pipeline.push(cmdBuf,transform2d);
    m_2d_pipeline.prepare_pass(cmdBuf,frame_descriptorSets);
    draw_mesh(cmdBuf, m_rend2d.m_gpu_mesh);

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

auto Renderer::present_image(
    u32 imageIndex,
    vk::Result acquire_res
)
-> void {
    auto present_rv = m_vkQueue.presentKHR(
        vk::PresentInfoKHR{}
            .setWaitSemaphores(*m_swapchain.renderFinishedSemaphores[imageIndex])
            .setSwapchains(*m_swapchain.descriptor)
            .setImageIndices(imageIndex)
    );
    if (   present_rv == vk::Result::eSuboptimalKHR 
        || present_rv == vk::Result::eErrorOutOfDateKHR
        || acquire_res == vk::Result::eSuboptimalKHR // since we ignore this in the previous check
        || m_requiresSwapchainRecreation
    ){
        recreate_swapchain();
    }else{
        ASSERT(present_rv==vk::Result::eSuccess);
    }
}

struct AcquiredImage{
    u32 imageIndex{0};
    vk::Result res;
    bool acquired () const{ return res != vk::Result::eErrorOutOfDateKHR; }
};
auto acquireNextImage(Swapchain const& swapchain, FrameData const& frame){
    // 2. Acquire the next image from the swapchain
    auto [res, imageIndex] = swapchain.descriptor.acquireNextImage(
        numeric_max<u64>,
        *frame.presentCompleteSemaphore
    );
    // only reset if we acquired an image from the swapchain, else we early returned
    return AcquiredImage{imageIndex, res};
}

void Renderer::draw(Camera const& cam) {
    auto const& frame = get_current_frame();
    auto fenceRV = m_vkDevice.waitForFences(*frame.fence, vk::True, numeric_max<u64>);
    if (fenceRV != vk::Result::eSuccess){
        LOG_FATAL("Failed to wait for fence");
    }

    auto const acquired = acquireNextImage(m_swapchain, frame);
    if (acquired.res == vk::Result::eErrorOutOfDateKHR){
        recreate_swapchain();
        return;
    }else{
        ASSERT(acquired.res ==vk::Result::eSuccess || acquired.res==vk::Result::eSuboptimalKHR);
    }
    auto imageIndex = acquired.imageIndex;
    m_vkDevice.resetFences(*frame.fence);


    update_ubo(frame, cam);
    record_commands(cam,frame,imageIndex);
    submit_commands(m_vkQueue, frame, *m_swapchain.renderFinishedSemaphores[imageIndex]);

    present_image(imageIndex, acquired.res);
    m_frameCount++;
}


