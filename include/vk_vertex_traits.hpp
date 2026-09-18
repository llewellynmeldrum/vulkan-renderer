#pragma once 
#include "vertex.hpp"
#include "vk_types.hpp"
#include "vertex_helpers.hpp"

template<typename T>
struct VertexTraits{
    static_assert(false, "Specializations must be explicitly created.");
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
        MAKE_VATTR_DESC(2, Vertex3D, uv_pos),
    };
};

template<> struct VertexTraits<Vertex2D>{
    static constexpr auto binding_desc = vk::VertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(Vertex2D),
        .inputRate = vk::VertexInputRate::eVertex
    };
    static constexpr auto attribute_desc = std::array{
        MAKE_VATTR_DESC(0, Vertex2D, pos),
        MAKE_VATTR_DESC(1, Vertex2D, color),
        MAKE_VATTR_DESC(2, Vertex2D, uv_pos),
        MAKE_VATTR_DESC(3, Vertex2D, shape_center_pos),
        MAKE_VATTR_DESC(4, Vertex2D, shapeID),
    };
};

// These are more helper structs which make use of the traits and whatnot

template<typename VertexType>
struct VertexInputState{
    static constexpr auto get()
    -> vk::PipelineVertexInputStateCreateInfo{
        return vk::PipelineVertexInputStateCreateInfo{}
        .setVertexBindingDescriptions(VertexTraits<VertexType>::binding_desc)
        .setVertexAttributeDescriptions(VertexTraits<VertexType>::attribute_desc);
    }
};

