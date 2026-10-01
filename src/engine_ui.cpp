#include "engine.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
auto Engine::ui_draw() -> void{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    {
        ImGui::ShowDemoWindow();
    }
    ImGui::Render();
}
