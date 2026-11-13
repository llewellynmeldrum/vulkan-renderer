#include "Gui.hpp"
#include "logger.hpp"
#include "vk_types.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>


Gui::Gui(SDL_Window* window){
    using detail::debug::get_c_handle;
    auto log_failure = [](std::string msg){
        LOG_FATAL("Failed to initialize imgui : {}",msg);
    };
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    if(! ImGui_ImplSDL3_InitForVulkan(window)){
        log_failure("ImGui_ImplSDL3_InitForVulkan failed.");
    }

    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.ScaleAllSizes(k_guiMainScale);
    style.FontScaleDpi = k_guiMainScale;
}
Gui::~Gui(){
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

auto Gui::wants_mouse() -> bool {
    return ImGui::GetIO().WantCaptureMouse;
}
auto Gui::wants_keyboard() -> bool {
    return ImGui::GetIO().WantCaptureMouse;
}
