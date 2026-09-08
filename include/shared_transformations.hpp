#pragma once 
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "types.hpp"
namespace detail{

inline constexpr auto logical_to_ndc(glm::vec2 pLogical, glm::vec2 winExtentLogical){
    // NOTE: This refers to the normal vulkan ndc, which is -1,-1 top left, +1,+1 bot right
    auto const p01 = glm::vec2{
        pLogical.x /winExtentLogical.x,
        pLogical.y / winExtentLogical.y,
    };
    auto const p02 = p01 * 2.0f;
    auto const ndc = p02 - 1.0f;
    return ndc;
}
inline constexpr auto ndc_to_logical(glm::vec2 pNdc, glm::vec2 winExtentLogical){
    auto const p02 = pNdc+1.0f; // 0,2
    auto const p01 = p02 / 2.0f; 
    auto const pLogical = glm::vec2{
        p01.x * winExtentLogical.x,
        p01.y * winExtentLogical.y,
    };
    return pLogical;

}
inline constexpr auto logical_to_pixel(glm::vec2 pLogical, f32 pixel_extent){
    return glm::vec2{
        pLogical.x * pixel_extent,
        pLogical.y * pixel_extent,
    };
}
inline constexpr auto pixel_to_logical(glm::vec2 pPixel, f32 pixel_extent){
    return glm::vec2{
        pPixel.x / pixel_extent,
        pPixel.y / pixel_extent,
    };
}
}// NAMESPACE: detail
