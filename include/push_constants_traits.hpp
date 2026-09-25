#pragma once
#include "vk_types.hpp"
#include "push_constants.hpp"
#include "common_utils.hpp"

namespace PushConstants{
#define DECL_PC_RANGE(T, stage)   \
    vk::PushConstantRange{              \
        .stageFlags=stage,              \
        .offset=0,                      \
        .size= SIZE_BITS(T)             \
    }                                    

template <typename T>
struct Traits{
    static_assert(false, "T is not a PushConstant Type (no Traits<T> specialization, check push_constant_traits.hpp)");
    static constexpr auto ranges = std::array<vk::PushConstantRange,0>{};
};
template<>
struct Traits<None>{
    static constexpr auto ranges = std::array<vk::PushConstantRange,0>{};
};

template<>
struct Traits<Transform2D>{
    static constexpr auto ranges = std::array{
        DECL_PC_RANGE(Transform2D, vk::ShaderStageFlagBits::eVertex)
    };
};

template<>
struct Traits<ModelMatrix>{
    static constexpr auto ranges = std::array{
        DECL_PC_RANGE(ModelMatrix, vk::ShaderStageFlagBits::eVertex)
    };
};
#undef DECL_PC_RANGE
} // namespace PushConstants
