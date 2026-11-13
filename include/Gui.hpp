#pragma once 

#include "cppslop.hpp"

#include <imgui.h>
FORWARD_DECLARE_STRUCT(Engine)
FORWARD_DECLARE_STRUCT(SDL_Window)
struct Gui{
    Gui(SDL_Window* window);
    ~Gui();
    auto wants_mouse() -> bool;
    auto wants_keyboard() -> bool;

    static constexpr auto k_guiMainScale = 1.5f;
};
