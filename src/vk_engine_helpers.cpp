#include "SDL3/SDL_video.h"
#include "vk_engine.hpp"
#include <thread>
void VkEngine::recreate_swapchain() {
    using namespace std::chrono_literals;
    auto framebuffer_sz = get_framebuffer_size();
    while (framebuffer_sz.width == 0 || framebuffer_sz.height == 0){
        // window is minimized 
        std::this_thread::sleep_for(10ms);
        handle_inputs();
        framebuffer_sz = get_framebuffer_size();
    }
    m_vkDevice.waitIdle();
    m_swapchain.descriptor.clear();
    init_swapchain();
}
void VkEngine::copy_buffer(vk::raii::Buffer const & src, vk::raii::Buffer &dst, vk::DeviceSize size){
    auto cmdCopyBuf = begin_single_use_cmd();
    cmdCopyBuf.copyBuffer(src,dst,vk::BufferCopy{.srcOffset=0, .dstOffset = 0, .size=size});
    end_single_use_cmd(std::move(cmdCopyBuf));
}
void VkEngine::copy_buffer_to_image(
    vk::raii::CommandBuffer const& cmd, 
    vk::raii::Buffer const& src_buffer, 
    vk::raii::Image const& dst_image,
    vk::Extent2D img_extent
){
    cmd.copyBufferToImage(
        src_buffer, 
        dst_image,
        vk::ImageLayout::eTransferDstOptimal,
        vk::BufferImageCopy{}
            .setBufferOffset(0)
            .setBufferRowLength(0)
            .setBufferImageHeight(0)
            .setImageSubresource(
                vk::ImageSubresourceLayers{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setMipLevel(0)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1)
            )
            .setImageOffset({0,0,0})
            .setImageExtent({img_extent.width,img_extent.height,1})

    );
}
void VkEngine::transition_img_layout(
    vk::raii::CommandBuffer const& cmd, 
    vk::raii::Image const& img,
    vk::ImageLayout old_layout,
    vk::ImageLayout new_layout
){
    auto barrier = vk::ImageMemoryBarrier{}
        .setOldLayout(old_layout)
        .setNewLayout(new_layout)
        .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setImage(img)
        .setSubresourceRange(
            vk::ImageSubresourceRange{}
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setLevelCount(1)
                .setLayerCount(1)
        )
    ;
    auto src_stage = vk::PipelineStageFlags{};
    auto dst_stage = vk::PipelineStageFlags{};

    if (old_layout == vk::ImageLayout::eUndefined 
        && 
        new_layout == vk::ImageLayout::eTransferDstOptimal
    ){
        barrier.setSrcAccessMask({});
        barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

        src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        dst_stage = vk::PipelineStageFlagBits::eTransfer;
    }else if (old_layout == vk::ImageLayout::eTransferDstOptimal
              && 
              new_layout == vk::ImageLayout::eShaderReadOnlyOptimal
    ){
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        src_stage = vk::PipelineStageFlagBits::eTransfer;
        dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
    }else {
        LOG_FATAL("Unsupported image layout transition ({})->({})",
                  vk::to_string(old_layout),vk::to_string(new_layout));
    }

    cmd.pipelineBarrier(src_stage, dst_stage, {}, {}, {}, barrier);
}

VkEngine& VkEngine::get_instance() { 
    return *m_loadedEngine; 
}
bool VkEngine::is_initialized() { 
    return m_window; 
}

u32 VkEngine::get_current_frame_index(){
    return m_frameCount % syncFrameCount;
}
FrameData& VkEngine::get_current_frame() {
    return m_inflightFrames.at(get_current_frame_index());
}
[[nodiscard]]
vk::Extent2D VkEngine::get_framebuffer_size() const noexcept{
    int w{},h{};
    SDL_GetWindowSizeInPixels(m_window, &w,&h);
    return vk::Extent2D{st_cast<u32>(w),st_cast<u32>(h)};
}
