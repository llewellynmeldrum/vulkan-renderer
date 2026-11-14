#include "SDL3/SDL_video.h"
#include "r3d.hpp"
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
void Renderer3D::cleanup() {
    if (is_initialized()) {
        m_gpuContext.await_idle();

        for (auto& [id, gpu_mesh]: m_gpu_meshes3d){
            gpu_mesh.clear();
        }

        cleanup_imgui();
        m_rendContext = {};
        m_gpuContext = {};

        m_rendContext.window = nullptr;
    }
}


