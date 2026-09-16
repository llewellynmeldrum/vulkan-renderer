#pragma once 

#include "types.hpp"
#include "glm_types.hpp"
// expects 8 bit r,g,b
// returns component-wise normalized (i.e [0,1])  glm::vec3 rgb
static constexpr inline auto make_rgb(u8 r,u8 g,u8 b) -> glm::vec3{
    return glm::vec3{r,g,b} / 255.0f;
}

// expects 8 bit r,g,b,a
// returns component-wise normalized (i.e [0,1])  glm::vec4 rgba
static constexpr inline auto make_rgba(u8 r,u8 g,u8 b, u8 a=255) -> glm::vec4{
    return glm::vec4{r,g,b,a} / 255.0f;
}
