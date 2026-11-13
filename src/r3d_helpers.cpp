
#include <thread>
#include "SDL3/SDL_video.h"
#include "glm_to_vk.hpp"
#include "renderer.hpp"
#include "vulkan/vulkan.hpp"
#include "renderer_init.hpp"
void Renderer3D::update_window_extent(glm::vec2 glm_windowLogicalExtent){
    m_rendContext.windowLogicalExtent = glm_windowLogicalExtent; 
    m_rendContext.windowPixelExtent = logical_to_pixel(glm_windowLogicalExtent);
    recreate_swapchain();
}

void Renderer3D::recreate_swapchain() {
    using namespace std::chrono_literals;
    m_gpuContext.device.waitIdle();
    m_gpuContext.swapchain.descriptor.clear();
    auto swap_settings = SwapchainSettings{
        .physical_device = m_gpuContext.physicalDevice,
        .device = m_gpuContext.device,
        .surface = m_gpuContext.surface,
    // BUG: 
    // is this a bug? Shouldnt you be asking SDL what the new framebuffer size is? Like when you first create the swap
    // chain, you call `get_framebuffer_size()`
        .extent_px = detail::to_vk_extent(m_rendContext.windowPixelExtent),
    };
    m_gpuContext.swapchain = detail::make_swapchain(swap_settings);
    // BUG:
    // need to recreate the depth attachment as well 
    m_rendContext.requiresSwapchainRecreation = false;
}

void Renderer3D::copy_buffer(vk::Buffer const & src, vk::Buffer const& dst, vk::DeviceSize size,vk::DeviceSize offset) const{
    auto cmdCopyBuf = begin_single_use_cmd();
    cmdCopyBuf.copyBuffer(src,dst,vk::BufferCopy{.srcOffset=offset, .dstOffset = 0, .size=size});
    end_single_use_cmd(std::move(cmdCopyBuf));
}

bool Renderer3D::is_initialized() { 
    return m_rendContext.window != nullptr; 
}

[[nodiscard]]
auto Renderer3D::get_current_frame_index()
const -> u32{
    return m_rendContext.frameCount % k_syncFrameCount;
}

[[nodiscard]]
vk::Extent2D Renderer3D::get_framebuffer_size() const noexcept{
    int w{},h{};
    SDL_GetWindowSizeInPixels(m_rendContext.window, &w,&h);
    return vk::Extent2D{st_cast<u32>(w),st_cast<u32>(h)};
}
auto Renderer3D::toggle_wireframe()
-> void{
    if (m_vkPolygonMode == vk::PolygonMode::eFill){
        m_vkPolygonMode = vk::PolygonMode::eLine;
    }
    else {
        m_vkPolygonMode = vk::PolygonMode::eFill;
    }
}
auto Renderer3D::get_viewport() const
-> vk::Viewport{
    return {
        m_rendContext.viewportOffset.x, m_rendContext.viewportOffset.y,
        m_rendContext.windowPixelExtent.x, m_rendContext.windowPixelExtent.y,
        k_depthMin, k_depthMax
    };
}
auto Renderer3D::get_scissor() const
-> vk::Rect2D{
    return {
        {
            st_cast<i32>(m_rendContext.viewportOffset.x),
            st_cast<i32>(m_rendContext.viewportOffset.y)
        },
        {
            st_cast<u32>(m_rendContext.windowPixelExtent.x),
            st_cast<u32>(m_rendContext.windowPixelExtent.y)
        },
    };
}
