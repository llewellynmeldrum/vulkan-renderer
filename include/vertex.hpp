#pragma once 

// to ensure that vulkan gets the expected alignment of structs we pass into slang land 

#include "glm_types.hpp"
#include "vk_types.hpp"
#include "vertex_helpers.hpp"
#include "vulkan/vulkan.hpp"

struct Vertex{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    static constexpr auto N_ATTRIBUTES{3uz};
};

template<typename T>
struct VertexTraits{
    static constexpr auto binding_desc = vk::VertexInputBindingDescription{};
    static constexpr auto attribute_desc = std::array<vk::VertexInputAttributeDescription,T::N_ATTRIBUTES>{};
};

template<> struct VertexTraits<Vertex>{
    static constexpr auto binding_desc = vk::VertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = vk::VertexInputRate::eVertex
    };
    static constexpr auto attribute_desc = std::array{
        MAKE_VATTR_DESC(0, Vertex, pos),
        MAKE_VATTR_DESC(1, Vertex, color),
        MAKE_VATTR_DESC(2, Vertex, texCoord),
    };
};



