#pragma once 

#include <cmath>

// BRIEF: wrapper around to cast a rounded value
template<typename RoundedType, typename InputType>
    requires std::integral<RoundedType> && std::floating_point<InputType>
inline constexpr auto round_to(InputType in) -> RoundedType{
    return static_cast<RoundedType>(std::round(in));
}

