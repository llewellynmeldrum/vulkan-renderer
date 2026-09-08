#pragma once 

#include "glm/detail/qualifier.hpp"
#include "glm/glm.hpp"
#include "vk_types.hpp"

namespace vtx_helpers{
    
// type trait for converting from C++ types to `vk::Format`. 
// Also contains member `slang_type_name`, for the equivalent type name in slang shader language.
// used by the `MAKE_VATTR_DESC` helper macro when defining vertex attribute descriptions
template<typename T>
struct vk_format_of;

// NOTE: INTEGRAL SCALAR TYPES
template<> struct vk_format_of<i8>{
    static constexpr auto value = vk::Format::eR8Sint;
    static constexpr auto slang_type_name = "int8_t";
};
template<> struct vk_format_of<i16>{
    static constexpr auto value = vk::Format::eR16Sint;
    static constexpr auto slang_type_name = "int16_t";
};
template<> struct vk_format_of<i32>{
    static constexpr auto value = vk::Format::eR32Sint;
    static constexpr auto slang_type_name = "int32_t";
};
template<> struct vk_format_of<i64>{
    static constexpr auto value = vk::Format::eR64Sint;
    static constexpr auto slang_type_name = "int64_t";
};
template<> struct vk_format_of<u8>{
    static constexpr auto value = vk::Format::eR8Uint;
    static constexpr auto slang_type_name = "uint8_t";
};
template<> struct vk_format_of<u16>{
    static constexpr auto value = vk::Format::eR16Uint;
    static constexpr auto slang_type_name = "uint16_t";
};
template<> struct vk_format_of<u32>{
    static constexpr auto value = vk::Format::eR32Uint;
    static constexpr auto slang_type_name = "uint32_t";
};
template<> struct vk_format_of<u64>{
    static constexpr auto value = vk::Format::eR64Uint;
    static constexpr auto slang_type_name = "uint64_t";
};

// NOTE: FLOATING POINT SCALAR TYPES
template<> struct vk_format_of<f64>{
    static constexpr auto value = vk::Format::eR64Sfloat;
    static constexpr auto slang_type_name = "double";
};
template<> struct vk_format_of<f32>{
    static constexpr auto value = vk::Format::eR32Sfloat;
    static constexpr auto slang_type_name = "float";
};

// NOTE: FLOATING POINT VEC TYPES
template<> struct vk_format_of<glm::vec2>{
    static constexpr auto value = vk::Format::eR32G32Sfloat;
    static constexpr auto slang_type_name = "float2";
};
template<> struct vk_format_of<glm::vec3>{
    static constexpr auto value = vk::Format::eR32G32B32Sfloat;
    static constexpr auto slang_type_name = "float3";
};
template<> struct vk_format_of<glm::vec4>{
    static constexpr auto value = vk::Format::eR32G32B32A32Sfloat;
    static constexpr auto slang_type_name = "float4";
};

// NOTE: INTEGRAL VEC TYPES
template<> struct vk_format_of<glm::ivec2>{
    static constexpr auto value = vk::Format::eR32G32Sint;
    static constexpr auto slang_type_name = "int2";
};
template<> struct vk_format_of<glm::ivec3>{
    static constexpr auto value = vk::Format::eR32G32B32Sint;
    static constexpr auto slang_type_name = "int3";
};
template<> struct vk_format_of<glm::ivec4>{
    static constexpr auto value = vk::Format::eR32G32B32A32Sint;
    static constexpr auto slang_type_name = "int4";
};

template<typename T>
static constexpr inline auto vk_format_of_v = vk_format_of<T>::value;

} // namespace vtx_helpers


namespace impl__{
#define MAKE_VATTR_DESC(LOC, T, M)\
    impl__::make_vattr_desc<decltype(T::M)>(LOC, offsetof(T,M))
template<typename T>
static inline consteval auto make_vattr_desc(u32 a_location, u32 a_offset){
    return vk::VertexInputAttributeDescription {
        .location = a_location,
        .binding = 0, 
        .format = vtx_helpers::vk_format_of_v<T>,
        .offset = a_offset
    };
}
} //namespace impl__
