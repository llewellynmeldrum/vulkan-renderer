#include <list>
#include <print>
#include <utility>
#include <variant>
#include <span>

#include "glm_types.hpp"
#include "engine.hpp"
#include "Engine_Events.hpp"
#include "Engine_Gui.hpp"

#include "color_utils.hpp"
#include "timer.hpp"
#include "auto_format.hpp"

#include "format_specs.hpp"

namespace Direction3{
    static constexpr auto Forward   =   glm::vec3{ 0.0f,  0.0f, -1.0f};
    static constexpr auto Backward  =   glm::vec3{ 0.0f,  0.0f, +1.0f};
    static constexpr auto Left      =   glm::vec3{-1.0f,  0.0f,  1.0f};
    static constexpr auto Right     =   glm::vec3{+1.0f,  0.0f,  0.0f};
    static constexpr auto Up        =   glm::vec3{ 1.0f, +1.0f,  0.0f};
    static constexpr auto Down      =   glm::vec3{ 0.0f, -1.0f,  1.0f};
};
void apply_actions_to_camera(std::span<Action const> actions, Camera & cam, f64 dt_s){
    // since dt is in seconds, the scale of the action is essentially the 'units per second'.
    for (auto const& act: actions){
        LOG_DBG(
            "Applying action '{}', {} {}",
            act.type,
            act.delta2==glm::vec2{0,0} ? "" : std::format("{}, ", act.delta2),
            act.delta3==glm::vec3{0,0,0} ? "" : std::format("{}", act.delta3)
        );
        auto scale = dt_s;
        switch (act.type){
            case ActionType::MoveCamera:   cam.move(act.get_delta3(), scale);       break;

            case ActionType::LookAround:   cam.look_around(act.get_delta2()); break;

            case ActionType::ZoomIn:       cam.zoom_in(scale);          break;
            case ActionType::ZoomOut:      cam.zoom_out(scale);          break;
        }
    }
}
struct ToggleButtonBind{
    ButtonCode button;
    Action action;
    auto operator()(InputState const& in)const -> std::optional<Action>{
        if (in.just_pressed(button)){
            return action;
        }else{
            return {};
        }
    }
};

struct HoldButtonBind{
    ButtonCode button;
    Action action;
    auto operator()(InputState const& in)const -> std::optional<Action>{
        if (in.is_down(button)){
            return action;
        }else{
            return {};
        }
    }
};
// for things like mouse movement, we need to evaluate the input state and see what the mouse delta is 
struct MouseMoveBind{
    auto operator()(InputState const& in) const-> std::optional<Action>{
        if (in.mouse_moved()){
            return Action{
                .type = ActionType::LookAround,
                .delta2 = in.mouse_movement_rel()
            };
        } else{
            return {};
        }
    }
};
template<typename T>
concept is_bind_object = requires(T const & bind, InputState const& in){
    {bind(in)} -> std::same_as<std::optional<Action>>;
};
static_assert(is_bind_object<MouseMoveBind>);
static_assert(!is_bind_object<int>);

using BindingType = std::variant<ToggleButtonBind, HoldButtonBind, MouseMoveBind>;
struct Binding{
    BindingType type;
    std::string name{}, desc{};
};
auto derive_actions_from_input(InputState const& input, std::span<const Binding> bindings, std::vector<Action>& out_actions) -> void{
    out_actions.clear();
    // For each binding, we check if the predicate input state has occured,
    // and if so, we push an action to the list.
    for (const auto& binding: bindings){
        auto action = std::visit(overloaded{
            [&](is_bind_object auto const& bind) -> std::optional<Action>{
                return bind(input);
            },
        },binding.type);

        if (action){
            out_actions.push_back(*action);
        }else{
            // no 
        }
    }
}
auto post_process_actions(std::vector<Action>& out_actions) -> void{
    auto new_actions = std::vector<Action>{};
    auto movement_delta_sum = glm::vec3{};
    for (auto& action: out_actions){
        if (action.type == ActionType::MoveCamera){
            movement_delta_sum += action.delta3; 
        }else{
            new_actions.push_back(action);
        }
    }
    if (movement_delta_sum != glm::vec3{0,0,0}){
    // We sum, and then normalize the movement actions into a single one.
    // This prevents diagonal movement from being faster than mono-direction movement,
    // and also avoids sending actions when two opposing keys are pressed (the sum_delta will be zero)
        new_actions.emplace_back(
            ActionType::MoveCamera,
            glm::vec2{0,0},
            glm::normalize(movement_delta_sum)
        );
    }
    out_actions = std::move(new_actions);
}

void Engine::loop(){
    // lets carve out the high level commands.
    auto dt_s = profiler.get_dt_seconds();
    if (ui.wants_mouse()){
        platform.uncapture_cursor();
    }else{
        platform.capture_cursor();
    }
    // TODO: consider merging the two functions, so that input state is just directly updated
    auto events = poll_events(platform);        // Platform polls for events
    update_input_states(                         // input is updated to reflect the events 
        input_state, 
        events, 
        &(m_shouldExit), 
        ui.wants_mouse(), 
        ui.wants_keyboard()
    );

    std::vector<Binding> bindings{
        Binding{ 
            .type = HoldButtonBind{
                .button = ButtonCode::KEY_W,
                .action = {ActionType::MoveCamera},
            }
        }
    };
    derive_actions_from_input(input_state,bindings, actions);
    post_process_actions(actions);
    apply_actions_to_camera(actions, cam, dt_s);

    // TODO: make the shit

//    auto frame = begin_frame();
//    draw_world(r3d,world);
//    draw_ui(r2d,ui);
}




Engine::Engine()
    : platform(initialWindowLogicalExtent)
    , input_state()
    , r3d(platform.get_window_handle(),initialWindowLogicalExtent,platform.m_pixelDensity)
    , world()
    , cam()
    , ui(platform.get_window_handle())
{
    init_gui(ui, platform, r3d);
}
