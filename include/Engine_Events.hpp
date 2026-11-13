#pragma once 
#include <glm/vec2.hpp>
#include "input.hpp"
#include "input_keycodes.hpp"
#include "platform.hpp"
struct PlatformQuitEvent    {};
struct MouseMoveEvent       { glm::vec2 pos; };
struct ScrollWheelMoveEvent { glm::vec2 diff; };
struct ButtonPressEvent     { ButtonCode code; };
struct ButtonReleaseEvent   { ButtonCode code; };

template<typename T>
auto is_mouse_button(T const& e) ->bool{
    if constexpr(same_as_any_nocvref<T,ButtonPressEvent,ButtonReleaseEvent>){
        return static_cast<i32>(e.code) <= static_cast<i32>(ButtonCode::MB_3);
    }else{
        static_assert(false,"Unknown event type for is_mouse_button.");
    }
}

using Event = std::variant<
    PlatformQuitEvent,
    MouseMoveEvent, 
    ScrollWheelMoveEvent, 
    ButtonPressEvent,
    ButtonReleaseEvent
>;

auto update_input_states(
    InputState& input,
    std::span<Event const> events,
    bool* shouldExit,
    bool shouldIgnoreMouse,
    bool shouldIgnoreKeyboard
) -> void;
auto poll_events(Platform& platform) -> std::vector<Event>;
