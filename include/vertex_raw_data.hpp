#pragma once 
#include <array>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include "vertex.hpp"

namespace vtx_raw_data{
inline constexpr auto ndc_triangle_verts = std::array{

    Vertex{.pos = {-1.0,-1.0 }, .color = {1.0, 0.0, 0.0}},
    Vertex{.pos = {+1.0,+1.0 }, .color = {0.0, 1.0, 0.0}},
    Vertex{.pos = {-1.0,+1.0 }, .color = {0.0, 0.0, 1.0}},

    Vertex{.pos = {+1.0,+1.0 }, .color = {1.0, 0.0, 0.0}},
    Vertex{.pos = {-1.0,-1.0 }, .color = {0.0, 1.0, 0.0}},
    Vertex{.pos = {+1.0,-1.0 }, .color = {0.0, 0.0, 1.0}},
};
//inline constexpr auto ndc_triangle_verts = std::array{
//    Vertex{.pos = {+0.0, -0.5}, .color = {1.0, 0.0, 0.0}},
//    Vertex{.pos = {+0.5, +0.5}, .color = {0.0, 1.0, 0.0}},
//    Vertex{.pos = {-0.5, +0.5}, .color = {0.0, 0.0, 1.0}},
//};
}
