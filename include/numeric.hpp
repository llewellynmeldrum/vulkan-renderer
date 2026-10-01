#pragma once 
template<typename T>
constexpr inline T numeric_min = std::numeric_limits<T>::lowest();

template<typename T>
constexpr inline T numeric_max = std::numeric_limits<T>::max();

constexpr static inline std::size_t N_CARDINAL_DIRECTIONS {4};

