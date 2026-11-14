#include <variant>

#include <glm/glm.hpp>

#include "cppslop.hpp"
#include "Engine_Events.hpp"

// NOTE:: Input pipeline:
// Subsytem     Inputs                                 Outputs
// (Platform)   SDL_PollEvents()                     = std::vector<Events>
//              std::vector<Events>                  = InputState
//              InputState + std::vector<Binding>    = std::vector<Action>
// Actions now can be handled by individual subsystems.

// This generates Events (eg. ButtonPressEvents) by polling SDL for ITS events.
// Another platform could plug in in their own implementation here.
auto poll_events(Platform& platform) -> std::vector<Event>{
    auto e = SDL_Event{};
    auto events = std::vector<Event>{};
    while (SDL_PollEvent(&e)){
        switch(e.type){
            case SDL_EventType::SDL_EVENT_QUIT:{
                events.emplace_back(
                    std::in_place_type<PlatformQuitEvent>
                );
            } break;

            case SDL_EventType::SDL_EVENT_KEY_DOWN:{
                events.emplace_back(
                    std::in_place_type<ButtonPressEvent>,
                    static_cast<ButtonCode>(e.key.scancode)
                );
            } break;

            case SDL_EventType::SDL_EVENT_KEY_UP:{
                events.emplace_back(
                    std::in_place_type<ButtonReleaseEvent>,
                    static_cast<ButtonCode>(e.key.scancode)
                );
            } break;
            case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_DOWN:{
                ASSERT(e.button.button <= 3);
                events.emplace_back(
                    std::in_place_type<ButtonPressEvent>,
                    static_cast<ButtonCode>(e.button.button)
                );
            } break;
            case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_UP:{
                ASSERT(e.button.button <= 3);
                events.emplace_back(
                    std::in_place_type<ButtonReleaseEvent>,
                    static_cast<ButtonCode>(e.button.button)
                );
            } break;
            case SDL_EventType::SDL_EVENT_MOUSE_MOTION:{
                events.emplace_back(
                    std::in_place_type<MouseMoveEvent>,
                    glm::vec2{e.motion.x, e.motion.y}
                );
            } break;
            case SDL_EventType::SDL_EVENT_MOUSE_WHEEL:{
                events.emplace_back(
                    std::in_place_type<ScrollWheelMoveEvent>,
                    glm::vec2{e.wheel.x, e.wheel.y}
                );
            } break;
        }
    }
    return events;
}

auto update_input_states(
    std::span<Event const> events,
    InputState& input,
    bool shouldIgnoreMouse,
    bool shouldIgnoreKeyboard
) -> InputState&{
    for (const auto& e: events){
        std::visit(overloaded{
            [&](PlatformQuitEvent const& ev){
                input.platform_quit_requested = true;
            },
            [&](MouseMoveEvent const& ev){
                if (shouldIgnoreMouse) return;
                input.mouse_pos = ev.pos;
                input.mouse_delta = ev.delta;
            }, 
            [&](ScrollWheelMoveEvent const& ev){
                if (shouldIgnoreMouse) return;
                input.scroll_wheel_delta = ev.delta;
            }, 
            [&](ButtonPressEvent const& ev){
                if (is_mouse_button(ev)){
                    if (shouldIgnoreMouse) return;
                }else {
                    if (shouldIgnoreKeyboard) return;
                }
                input.pressed.at(ev.code) = true;
            },
            [&](ButtonReleaseEvent const& ev){
                // NOTE: Release events are never ignored.
                // If they were, the ui grabbing control after a press event 
                // will mean that future release events are dropped (if they used the same filtering 
                // as ButtonPressEvent.)
                input.pressed.at(ev.code) = false;
            },
        }, e);
    }
    return input;
}
