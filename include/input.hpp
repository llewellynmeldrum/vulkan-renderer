#pragma once
#include <queue>
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <glm/vec2.hpp>

#include "input_keycodes.hpp"
#include "logger.hpp"
#include "concept_helpers.hpp"
#include "enum_map.hpp"
#include "transformations.hpp"

#include "enum_map.hpp"


struct InputState{
public:
    EnumMap<ButtonCode, bool> pressed{};
    EnumMap<ButtonCode, bool> pressed_previous{};
    auto is_down(ButtonCode c) const -> bool{
        return pressed.at(c); 
    }
    auto just_pressed(ButtonCode c) const -> bool{
        return !pressed_previous.at(c) && pressed.at(c); 
    }
    auto end_frame() -> void{
        for (auto i = 0uz; i<pressed.size(); i++){
            auto code = static_cast<ButtonCode>(i);
            pressed_previous.at(code) = pressed.at(code);
        }
        mouse_pos_previous = mouse_pos;
        mouse_pos={};

        mouse_wheel_relative={};
    }

    auto mouse_moved() const -> bool{
        return mouse_pos != mouse_pos_previous;
    }
    auto scroll_wheel_moved()const  -> bool{
        return mouse_wheel_relative != glm::vec2{0,0};
    }
    glm::vec2 mouse_pos{};
    glm::vec2 mouse_pos_previous{};
    auto mouse_movement_rel() const -> glm::vec2{
        return mouse_pos - mouse_pos_previous;
    }
    glm::vec2 mouse_wheel_relative{};
};
