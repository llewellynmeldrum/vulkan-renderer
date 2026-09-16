#pragma once 
#include <array>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include "vertex.hpp"
#include "vulkan/vulkan.hpp"

namespace vtx_raw_data{
static constexpr u32 vtx_count(std::span<const Vertex3D> vertices){
    return static_cast<u32>(vertices.size());
}
static constexpr u32 idx_count(std::ranges::contiguous_range auto indices){
    return static_cast<u32>(indices.size());
}

template<typename T>
static constexpr vk::IndexType vk_IndexType(){
    if constexpr (std::same_as<T,u16>){
        return vk::IndexType::eUint16;
    }else if constexpr (std::same_as<T,u8>){
        return vk::IndexType::eUint8;
    }else if constexpr (std::same_as<T,u32>){
        return vk::IndexType::eUint32;
    }else{
        static_assert(false, "Unknown index type");
    }
}


static constexpr inline Vertex3D TL{.pos={-0.5,-0.5, 0.0}, .color={1.0,0.0,0.0}, .texCoord={0.0,1.0}};
static constexpr inline Vertex3D BL{.pos={-0.5,+0.5, 0.0}, .color={0.0,1.0,0.0}, .texCoord={0.0,0.0}};
static constexpr inline Vertex3D TR{.pos={+0.5,-0.5, 0.0}, .color={0.0,0.0,1.0}, .texCoord={1.0,1.0}};
static constexpr inline Vertex3D BR{.pos={+0.5,+0.5, 0.0}, .color={1.0,1.0,1.0}, .texCoord={1.0,0.0}};

// i wouldnt mind making some sort of draw wireframe function.

inline constexpr auto ccw_quad_verts = std::array{
    BL,BR,
    TR,TL,
};
// ccw winding starting in bot left
inline constexpr auto ccw_quad_indices = std::array<u32,6>{
    0uz, 1uz, 2uz, 
    2uz, 3uz, 0uz,
};

}// namespace vtx_raw_data
