#pragma once 

// to ensure that vulkan gets the expected alignment of structs we pass into slang land 

#include "glm_types.hpp"
#include "vk_types.hpp"

struct Vertex{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    static constexpr auto N_ATTRIBUTES{3uz};
};



