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
        m_vkDescriptorSetLayout.clear();
        m_vkPipelineLayout.clear();
        m_vkPipeline.clear();

        m_vertexBuffer.clear();
        m_vertexBufferMemory.clear();

        m_vkDescriptorSets.clear();
        m_vkDescriptorPool.clear();

        m_indexBuffer.clear();
        m_indexBufferMemory.clear();

        m_vkDevice.clear();
        m_vkPhysicalDevice.clear();
        m_vkSurface.clear();
        // this cant be done here, shouldnt it happen after destruction of raii stuff?
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    m_loadedEngine = nullptr;
}


void VkEngine::cleanup_window() const noexcept{
    SDL_DestroyWindow(m_window);
}
