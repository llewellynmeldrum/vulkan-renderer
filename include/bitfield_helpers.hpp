#pragma once 
#include "primitive_types.hpp"
// NOTE: bitfield helpers, with auto enum -> underlying conversions.
// this is to allow for support for bitfields that use the vulkan raii style,
// i.e XXXFlagBits + XXXFlags combo

// Tests whether or not a bitfield contains AT LEAST the flags in `set`
template<typename T>
inline constexpr bool bitfield_has_all(u32 bitfield, T set){
    u32 set_bits{};
    if constexpr (std::is_enum_v<T>){
        set_bits = std::to_underlying(set);
    }else{
        set_bits = set;
    }
    return (bitfield & set_bits) == bitfield;
}

// Tests whether or not a bitfield contains ANY of the flags in `set`
// eg: bitfield_has_any(keymodbits, ctrl_bits) -> are any of the ctrl bits set (non-exclusive.)
template<typename T> 
inline constexpr bool bitfield_has_any(u32 bitfield, T set){
    u32 set_bits{};
    if constexpr (std::is_enum_v<T>){
        set_bits = std::to_underlying(set);
    }else{
        set_bits = set;
    }
    return (bitfield & set_bits) == 0 ? false : true;
}

// Tests whether or not a bitfield contains ONLY the flags in set
// eg: bitfield_has_any(keymodbits, ctrl_bits) -> are ONLY the ctrl bits set (exclusive.)
template<typename T>
inline constexpr bool bitfield_has_only(u32 bitfield, T set){
    u32 set_bits{};
    if constexpr (std::is_enum_v<T>){
        set_bits = std::to_underlying(set);
    }else{
        set_bits = set;
    }
    return (bitfield == set_bits);
}

// NOTE: STATIC TESTS 
// these probably shouldnt be in the main include, as every file which includes them will probably evaluate them.
// But they take MAYBE 200ns to evaluate. should be fine for these, but other, more involved static assert style
// testing (if its like an exhaustive consteval function or something) should live in another isolated cpp TU
inline static constexpr u32 A = 0b0011;
inline static constexpr u32 none_set = 0b0000;
inline static constexpr u32 one_set= 0b0001;
inline static constexpr u32 two_set = 0b0010;
inline static constexpr u32 both_set = 0b0011;
inline static constexpr u32 more_set = 0b1111;

static_assert(!bitfield_has_all(A, none_set));
static_assert(!bitfield_has_all(A, one_set));
static_assert(!bitfield_has_all(A, two_set));
static_assert( bitfield_has_all(A, both_set));
static_assert( bitfield_has_all(A, more_set));

static_assert(!bitfield_has_any(A, none_set));
static_assert( bitfield_has_any(A, one_set));
static_assert( bitfield_has_any(A, two_set));
static_assert( bitfield_has_any(A, both_set));
static_assert( bitfield_has_any(A, more_set));

static_assert(!bitfield_has_only(A, none_set));
static_assert(!bitfield_has_only(A, one_set));
static_assert(!bitfield_has_only(A, two_set));
static_assert( bitfield_has_only(A, both_set));
static_assert(!bitfield_has_only(A, more_set));
