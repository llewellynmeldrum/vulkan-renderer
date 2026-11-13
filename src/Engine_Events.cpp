#include <variant>

#include <glm/glm.hpp>

#include "cppslop.hpp"
#include "Engine_Events.hpp"

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
                ASSERT(e.button.button <= 3);
                events.emplace_back(
                    std::in_place_type<MouseMoveEvent>,
                    glm::vec2{e.motion.x, e.motion.y}
                );
            } break;
            case SDL_EventType::SDL_EVENT_MOUSE_WHEEL:{
                ASSERT(e.button.button <= 3);
                events.emplace_back(
                    std::in_place_type<MouseMoveEvent>,
                    glm::vec2{e.wheel.x, e.wheel.y}
                );
            } break;
        }
    }
    return events;
}

auto update_input_states(
    InputState& input,
    std::span<Event const> events,
    bool* shouldExit,
    bool shouldIgnoreMouse,
    bool shouldIgnoreKeyboard
) -> void{
    for (const auto& e: events){
        std::visit(overloaded{
            [&](PlatformQuitEvent const& ev){
                *shouldExit = true;
            },
            [&](MouseMoveEvent const& ev){
                if (shouldIgnoreMouse) return;
                input.mouse_pos = ev.pos;
            }, 
            [&](ScrollWheelMoveEvent const& ev){
                if (shouldIgnoreMouse) return;
                input.mouse_wheel_relative = ev.diff;
            }, 
            [&](ButtonPressEvent const& ev){
                if (is_mouse_button(ev) && shouldIgnoreMouse){
                    return;
                }else if (shouldIgnoreKeyboard){
                    return;
                }
                input.pressed.at(ev.code) = true;
            },
            [&](ButtonReleaseEvent const& ev){
                if (is_mouse_button(ev) && shouldIgnoreMouse){
                    return;
                }else if (shouldIgnoreKeyboard){
                    return;
                }
                input.pressed.at(ev.code) = false;
            },
        }, e);
    }
}
