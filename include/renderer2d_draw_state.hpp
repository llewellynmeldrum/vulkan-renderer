#pragma once 
#include "types.hpp"
#include "glm_types.hpp"
struct DrawState{
    glm::vec4 outline_color{};
    glm::vec4 fill_color{};
    f32 font_size{10.0f};
    f32 outline_thickness_px{10.0f};
};

