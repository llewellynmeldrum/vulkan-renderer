#include <cstddef>
#include <ranges>
#include <thread>

#include "vertex_raw_data.hpp"
#include "vk_managed_buffers.hpp"
#include "renderer.hpp"
#include "vk_types.hpp"
#include "vk_util.hpp"

auto upload_model_matrix(
    void* ubo_mapped_memory,
    glm::mat4x4 model
) -> void {
    auto dst_offset = offsetof(UniformBufferObject, model);
    void* src = &model;
    void* dst = reinterpret_cast<std::byte*>(ubo_mapped_memory)+ dst_offset;
    memcpy(dst, src, sizeof(UniformBufferObject::model));
}
auto upload_ubo(
    void* ubo_mapped_memory,
    UniformBufferObject ubo
) -> void {
    // HACK: glm uses y up for clip space, vulkan uses y down. flip here
    ubo.proj[1][1] *= -1; 
    memcpy(ubo_mapped_memory, &ubo, sizeof(ubo));
}

auto Renderer::update_ubo(
    FrameData const& frame,
    Camera const& cam,
    glm::mat4x4 model_matrix
) -> void {
    auto aspect = m_swapchain.extent.width / static_cast<f32>( m_swapchain.extent.height);
    auto ubo = UniformBufferObject{
        .model = model_matrix,
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

void record_mesh_draw_cmds(vk::raii::CommandBuffer const& cmdBuf, GpuMesh const& gpu_mesh){
    ASSERT(gpu_mesh.m_vertex_count!= 0);
    cmdBuf.bindVertexBuffers(0, gpu_mesh.vertices.buffer, {0});

    ASSERT(gpu_mesh.m_index_count != 0);
    cmdBuf.bindIndexBuffer(gpu_mesh.indices.buffer, 0, gpu_mesh.index_type);

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
)
-> void {
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


    for (const auto& [id, gpu_mesh]: m_gpu_meshes){
        LOG_DBG("Drawing mesh id={}, vtx:{},idx:{}",id,gpu_mesh.m_vertex_count, gpu_mesh.m_index_count);
        auto model_matrix = m_model_matrices.at(id);
        upload_model_matrix(frame.uniformBufferMappedMemory, model_matrix);
        draw_mesh_pass(gpu_mesh, cmdBuf,m_fill_pipeline,frameIndex,vk::PolygonMode::eFill);
        draw_mesh_pass(gpu_mesh, cmdBuf,m_line_pipeline,frameIndex,vk::PolygonMode::eLine);
    }

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
auto Renderer::draw_mesh_pass(
    GpuMesh const& gpu_mesh,
    vk::raii::CommandBuffer const& cmdBuf,
    ShaderPipelineContext const&  pipeline, 
    u32 frameIndex, 
    vk::PolygonMode poly_mode
) -> void{
    cmdBuf.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        *pipeline.vk_pipeline
    );
    cmdBuf.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        pipeline.layout,
        0, 
        *m_vkDescriptorSets[frameIndex],
        nullptr
    );
    cmdBuf.setPolygonModeEXT(poly_mode);

    record_mesh_draw_cmds(cmdBuf, gpu_mesh);
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


    update_ubo(frame,cam,glm::mat4(1.0f));
    record_commands(cam,frame,imageIndex);
    submit_commands(m_vkQueue, frame, *m_swapchain.renderFinishedSemaphores[imageIndex]);

    present_image(imageIndex, acquired.res);
    m_frameCount++;
}


