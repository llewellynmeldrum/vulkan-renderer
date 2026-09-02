#pragma once 
#include "types.hpp"
template<typename T>
struct MinMax{
    T min{},  max{};
    auto range() const noexcept {return max-min;}
    auto mid() const noexcept {return min + (range()/2.0f);}
    // linear interpolation over the range, 0 -> min, 1-> max, 0.5 = min+ ( (max-min)*0.5) 
    auto map01(f32 zero_to_one) const noexcept{
        ASSERT(zero_to_one >= 0.0f);
        ASSERT(zero_to_one <= 1.0f);
        return min + (zero_to_one * range());
    }
    auto unlerp(T val) const noexcept{
        return (val - min)/range();
    }
};
