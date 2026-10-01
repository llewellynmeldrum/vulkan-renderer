#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus

#include <cmath>
#include "primitive_types.hpp"
#include <stack>
#include <deque>
#include <ranges>

#define st_cast static_cast
#define NOP ((void)0)
// The ctor that never was
template<typename T, typename Container=std::deque<T>>
constexpr inline auto init_stack(std::initializer_list<T> in){
    return std::stack<T,Container>(std::from_range, in);
}
static constexpr auto divisible_by(f32 a, f32 b){
    auto a_int = static_cast<i32>(std::round(a));
    auto b_int = static_cast<i32>(std::round(b));
    return a_int % b_int == 0;
}

// static_cast wrapper because i have lazy fingers
template<typename To, typename From>
[[nodiscard]] constexpr inline auto cast(From&& f) noexcept -> To{
    return static_cast<To>(std::forward<From>(f));
}

// NOTE: I know this is considered bad practice, and I probably wont do it for the ""s operators, but i think ""sv is unique enough that it shouldnt matter.
// I just really dislike c strings.
using namespace std::string_view_literals;
