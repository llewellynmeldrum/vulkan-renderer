#pragma once 
#include "vertex.hpp"
#include "vk_types.hpp"
#include "vertex_helpers.hpp"

template<typename T>
struct VertexTraits{
    static constexpr auto binding_desc = vk::VertexInputBindingDescription{};
    static constexpr auto attribute_desc = std::array<vk::VertexInputAttributeDescription,T::N_ATTRIBUTES>{};
};

template<> struct VertexTraits<Vertex3D>{
    static constexpr auto binding_desc = vk::VertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(Vertex3D),
        .inputRate = vk::VertexInputRate::eVertex
    };
    static constexpr auto attribute_desc = std::array{
        MAKE_VATTR_DESC(0, Vertex3D, pos),
        MAKE_VATTR_DESC(1, Vertex3D, color),
        MAKE_VATTR_DESC(2, Vertex3D, texCoord),
    };
};

