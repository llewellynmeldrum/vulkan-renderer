#pragma once 
#include "input.hpp"
#include "input_actions.hpp"
#include "input_keycodes.hpp"
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
                .delta2 = in.mouse_delta,
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

struct Binding{
    using BindingType = std::variant<ToggleButtonBind, HoldButtonBind, MouseMoveBind>;
    BindingType type;
    std::string name{}, desc{};
};
