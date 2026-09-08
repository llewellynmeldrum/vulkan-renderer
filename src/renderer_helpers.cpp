
#include <thread>
#include "SDL3/SDL_video.h"
#include "glm_to_vk.hpp"
#include "renderer.hpp"
#include "vulkan/vulkan.hpp"
#include "renderer_init.hpp"
void Renderer::handle_window_resize(glm::vec2 glm_windowLogicalExtent){
    m_windowLogicalExtent = glm_windowLogicalExtent; 
    m_windowPixelExtent = logical_to_pixel(glm_windowLogicalExtent);
    recreate_swapchain();
}

void Renderer::recreate_swapchain() {
    using namespace std::chrono_literals;
    m_vkDevice.waitIdle();
    m_swapchain.descriptor.clear();
    auto swap_settings = SwapchainSettings{
        .physical_device = m_vkPhysicalDevice,
        .device = m_vkDevice,
        .surface = m_vkSurface,
        .extent_px = detail::to_vk_extent(m_windowPixelExtent),
    };
    m_swapchain = detail::make_swapchain(swap_settings);
    // BUG:
    // need to recreate the depth attachment as well 
    m_requiresSwapchainRecreation = false;
}

void Renderer::copy_buffer(vk::Buffer const & src, vk::Buffer const& dst, vk::DeviceSize size,vk::DeviceSize offset){
    auto cmdCopyBuf = begin_single_use_cmd();
    cmdCopyBuf.copyBuffer(src,dst,vk::BufferCopy{.srcOffset=offset, .dstOffset = 0, .size=size});
    end_single_use_cmd(std::move(cmdCopyBuf));
}

bool Renderer::is_initialized() { 
    return m_window; 
}

u32 Renderer::get_current_frame_index(){
    return m_frameCount % k_syncFrameCount;
}
FrameData& Renderer::get_current_frame() {
    return m_inflightFrames.at(get_current_frame_index());
}
[[nodiscard]]
vk::Extent2D Renderer::get_framebuffer_size() const noexcept{
    int w{},h{};
    SDL_GetWindowSizeInPixels(m_window, &w,&h);
    return vk::Extent2D{st_cast<u32>(w),st_cast<u32>(h)};
}
auto Renderer::toggle_wireframe()
-> void{
    if (m_vkPolygonMode == vk::PolygonMode::eFill){
        m_vkPolygonMode = vk::PolygonMode::eLine;
    }
    else {
        m_vkPolygonMode = vk::PolygonMode::eFill;
    }
}
auto Renderer::get_viewport() const
-> vk::Viewport{
    return {
        m_viewportOffset.x, m_viewportOffset.y,
        m_windowPixelExtent.x, m_windowPixelExtent.x,
        k_depthMin, k_depthMax
    };
}
auto Renderer::get_scissor() const
-> vk::Rect2D{
    return {
        {
            st_cast<i32>(m_viewportOffset.x),
            st_cast<i32>(m_viewportOffset.y)
        },
        {
            st_cast<u32>(m_windowPixelExtent.x),
            st_cast<u32>(m_windowPixelExtent.y)
        },
    };
}
