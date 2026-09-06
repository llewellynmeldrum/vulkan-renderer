#include "SDL3/SDL_video.h"
#include "vk_engine.hpp"
void VkEngine::cleanup() {
    if (is_initialized()) {
        m_vkDevice.waitIdle();

        m_swapchain = Swapchain{};
        for (auto& frame: m_inflightFrames){
            frame = FrameData{};
        }
        m_vkQueue.clear();
        m_line_pipeline.clear();
        m_fill_pipeline.clear();

        m_gpu_heightmapMesh.clear();


        m_vkDescriptorPool.clear();


        m_vkDevice.clear();
        m_vkPhysicalDevice.clear();
        m_vkSurface.clear();

        m_gpu_heightmapMesh.clear();

        cleanup_vma();
        cleanup_window();
        m_window = nullptr;
    }
}


void VkEngine::cleanup_window() const noexcept{
    SDL_DestroyWindow(m_window);
}
