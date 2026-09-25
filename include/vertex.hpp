#pragma once 

// to ensure that vulkan gets the expected alignment of structs we pass into slang land 

#include "glm_types.hpp"
#include "vk_types.hpp"
#include "cpp_slang_shared.hpp"

struct Vertex3D{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv_pos;
    static constexpr auto N_ATTRIBUTES{3uz};
};

struct Vertex2D{
    glm::vec2 pos;
    glm::vec4 color;
    glm::vec2 uv_pos;
    glm::vec2 shape_center_pos{};
    u32 shapeID{0};

    static constexpr auto N_ATTRIBUTES{3uz};
};





