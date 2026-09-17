#pragma once 
#include <stack>
#include <deque>
#include <ranges>
// The ctor that never was
template<typename T, typename Container=std::deque<T>>
constexpr inline auto init_stack(std::initializer_list<T> in){
    return std::stack<T,Container>(std::from_range, in);
}
