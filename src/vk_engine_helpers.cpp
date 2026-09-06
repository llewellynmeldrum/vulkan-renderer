
#include <thread>
#include "SDL3/SDL_video.h"
#include "vk_engine.hpp"
#include "vulkan/vulkan.hpp"
#include "vk_engine_init.hpp"
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
    auto swap_settings = SwapchainSettings{
        .physical_device = m_vkPhysicalDevice,
        .device = m_vkDevice,
        .surface = m_vkSurface,
        .extent_px = get_framebuffer_size(),
    };
    m_swapchain = detail::make_swapchain(swap_settings);
}
void VkEngine::copy_buffer(vk::Buffer const & src, vk::Buffer const& dst, vk::DeviceSize size,vk::DeviceSize offset){
    auto cmdCopyBuf = begin_single_use_cmd();
    cmdCopyBuf.copyBuffer(src,dst,vk::BufferCopy{.srcOffset=offset, .dstOffset = 0, .size=size});
    end_single_use_cmd(std::move(cmdCopyBuf));
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
