#pragma once 
#include "glm_types.hpp"
#include "vk_types.hpp"
namespace detail{
template<typename Vec>
auto to_vk_extent(Vec vec){
    if constexpr (std::same_as<Vec, glm::vec2>){
        return vk::Extent2D(vec.x,vec.y);
    }else if constexpr (std::same_as<Vec,glm::vec3>){
        return vk::Extent3D(vec.x,vec.y,vec.z);
    }

}
} // namespace detail
