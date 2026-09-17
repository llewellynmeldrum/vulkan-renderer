#pragma once 

#include <concepts>
#include "push_constants_traits.hpp"

namespace PushConstants{

// Valid push constant types have specialized the Traits struct and defined a range.
// Use PushConstants::None if your pipeline doesnt use them.
template<typename T>
concept is_valid = requires{
    Traits<T>::ranges;
};


} //namespace PushConstants
