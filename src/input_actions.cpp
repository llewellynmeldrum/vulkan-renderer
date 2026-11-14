
// NOTE: We pass in the actions vector to reuse its allocation
#include "input_binds.hpp"
#include "input.hpp"
#include "input_actions.hpp"

auto derive_actions_from_input(
    std::vector<Action>& out_actions,
    InputState const& input,
    std::span<const Binding> bindings
) -> std::vector<Action>&{
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
    return out_actions;
}
auto post_process_actions(std::vector<Action>& out_actions) -> void{
    auto new_actions = std::vector<Action>{};
    auto movement_delta_sum = glm::vec3{};
    LOG_DBG("Actions before post processing: {}",out_actions.size());
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
    LOG_DBG("Actions AFTER post processing: {}",out_actions.size());
}
