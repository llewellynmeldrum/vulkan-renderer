#pragma once 
#include <span>
#include <cstddef>

template<typename T, std::size_t extent=std::dynamic_extent> 
using const_span = std::span<const T, extent>;

template<typename T> 
using dynamic_span = std::span<T, std::dynamic_extent>;

template<typename T> 
using const_dyn_span = std::span<const T, std::dynamic_extent>;
