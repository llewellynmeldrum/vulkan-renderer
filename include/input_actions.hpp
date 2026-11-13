#pragma once
#include "glm_types.hpp"
#include "libassert/assert.hpp"
#include "auto_format.hpp"
enum struct ActionType{
    MoveCamera,
    LookAround,

    ZoomIn,
    ZoomOut,
};
struct Action{
    ActionType type;
    glm::vec2 delta2{0,0};      // Only applicable for certain actions, i.e the look actions.
    glm::vec3 delta3{0,0,0};    // Applicable for movement
    auto get_delta3() const -> glm::vec3{
        ASSERT(type == ActionType::MoveCamera);
        return delta3;
    }
    auto get_delta2() const -> glm::vec2{
        ASSERT(type == ActionType::LookAround);
        return delta2;
    }
};
