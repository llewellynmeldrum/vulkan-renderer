#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_sdl3.h>
#include <vulkan/vulkan_core.h>

#include "r3d.hpp"
#include "vk_types.hpp"
void Renderer3D::record_imgui_commands(vk::raii::CommandBuffer const& cmdBuf){
    if (auto* draw_data = ImGui::GetDrawData()){
       ImGui_ImplVulkan_RenderDrawData(draw_data,*cmdBuf);
    }
}
void Renderer3D::cleanup_imgui(){
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
