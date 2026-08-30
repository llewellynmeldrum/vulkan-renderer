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
    auto allocInfo = vk::CommandBufferAllocateInfo{
        .commandPool = *get_current_frame().commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };

    auto cmdCopyBuf = std::move(m_vkDevice.allocateCommandBuffers(allocInfo).front());

    cmdCopyBuf.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    cmdCopyBuf.copyBuffer(src,dst,vk::BufferCopy{.srcOffset=0, .dstOffset = 0, .size=size});
    cmdCopyBuf.end();

    m_vkQueue.submit(
        vk::SubmitInfo{}
            .setCommandBuffers(*cmdCopyBuf),
        nullptr
    );
    // We wait to ensure the transfer happened
    m_vkQueue.waitIdle();
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
