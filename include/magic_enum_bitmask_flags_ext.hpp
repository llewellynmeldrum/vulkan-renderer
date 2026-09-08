#pragma once 

#include "magic_enum.hpp"
#include "bitmask_flags.hpp"
#include "common_concepts.hpp"

namespace magic_enum::ext{
    template<typename E>
    inline constexpr std::size_t enum_count(){
        if constexpr (is_enum<E>){
            using enum_t = E;
            return magic_enum::enum_count<enum_t>();
        }else if constexpr (is_bitmask_flag<E>){
            using enum_t = E::underlying_t;
            return magic_enum::enum_count<enum_t>();
        }else{
            static_assert(false,"Neither a regular enum or bitmask flag type.");
        }
    }
};
