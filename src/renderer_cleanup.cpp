#include "SDL3/SDL_video.h"
#include "renderer.hpp"
void Renderer::cleanup() {
    if (is_initialized()) {
        m_vkDevice.waitIdle();

        m_swapchain = Swapchain{};
        for (auto& frame: m_inflightFrames){
            frame = FrameData{};
        }
        m_vkQueue.clear();
        m_line_pipeline.clear();
        m_fill_pipeline.clear();



        m_vkDescriptorPool.clear();


        m_vkDevice.clear();
        m_vkPhysicalDevice.clear();
        m_vkSurface.clear();

        for (auto& [id, gpu_mesh]: m_gpu_meshes){
            gpu_mesh.clear();
        }

        cleanup_vma();
        m_window = nullptr;
    }
}


