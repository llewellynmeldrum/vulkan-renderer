#include "SDL3/SDL_video.h"
#include "renderer.hpp"
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
void Renderer::cleanup() {
    if (is_initialized()) {
        m_vkDevice.waitIdle();


        cleanup_imgui();
        m_swapchain = Swapchain{};
        for (auto& frame: m_inflightFrames){
            frame = FrameData{};
        }
        m_vkQueue.clear();
        m_line_pipeline.clear();
        m_2d_pipeline.clear();
        m_fill_pipeline.clear();


        m_vkDescriptorPool.clear();


        m_vkDevice.clear();
        m_vkPhysicalDevice.clear();
        m_vkSurface.clear();

        for (auto& [id, gpu_mesh]: m_gpu_meshes3d){
            gpu_mesh.clear();
        }
        m_rend2d.clear();

        cleanup_vma();
        m_window = nullptr;
    }
}


