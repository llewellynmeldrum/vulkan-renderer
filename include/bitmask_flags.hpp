#pragma once 
#include "common_concepts.hpp"
template<typename E> requires is_enum<E>
struct Flags{
    using underlying_t = std::underlying_type_t<E>;
    constexpr Flags() = default;
    constexpr Flags(E e) : m_bits(std::to_underlying(e)){}
    constexpr explicit Flags(underlying_t raw) : m_bits(raw){}

    constexpr underlying_t raw() const noexcept{ return m_bits;}

    constexpr bool has(E e) const{ return (m_bits & std::to_underlying(e)) == std::to_underlying(e); };
    constexpr inline Flags& operator|=(Flags rhs){ return *this = (*this | rhs); }
    constexpr inline Flags& operator&=(Flags rhs){ return *this = (*this & rhs); }
    constexpr inline Flags& operator^=(Flags rhs){ return *this = (*this ^ rhs); }

    constexpr inline Flags operator~() const{ return Flags(~m_bits); }

    underlying_t m_bits{0};
};

template<typename E>
    requires is_enum<E>
constexpr inline auto operator|(Flags<E> lhs, Flags<E> rhs){
    return Flags<E>(std::to_underlying(lhs) | std::to_underlying(rhs));
};
template<typename E>
    requires is_enum<E>
constexpr inline auto operator&(Flags<E> lhs, Flags<E> rhs){
    return Flags<E>(std::to_underlying(lhs) & std::to_underlying(rhs));
};
template<typename E>
    requires is_enum<E>
constexpr inline auto operator^(Flags<E> lhs, Flags<E> rhs){
    return Flags<E>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
};

template<typename T>
struct enum_bitwise_or_opt_in{
    static constexpr bool enabled = false;
};

template<typename E>
    requires is_enum<E> && enum_bitwise_or_opt_in<E>::enabled
constexpr inline auto operator|(E lhs, E rhs){
    return Flags<E>(std::to_underlying(lhs) | std::to_underlying(rhs));
};


template<typename T>
concept is_bitmask_flag = requires(T t){
    typename T::underlying_t;
    t.m_bits;
    requires std::same_as<decltype(t.m_bits),typename T::underlying_t>;
};
