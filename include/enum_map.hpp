#pragma once
#include <type_traits>
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
#include <utility>
#include <array>
#include <initializer_list>

#include "concept_helpers.hpp"
#include "bitmask_flags.hpp"
#include "common.hpp"
#include "meta_helpers.hpp"
#include "math_helpers.hpp"

enum struct vals{
    a = 1 << 0,
    b = 1 << 1,
    c = 1 << 2,
};
namespace detail{
#if CPP26
    template<typename T>
    // returns the first defined element of an enumerator
    consteval auto enum_count() -> std::size_t{
        return std::meta::enumerators_of(^^T).size();
    }
    template<typename T>
    consteval auto enum_is_bitmap() -> bool{
        using underlying_t = std::underlying_type_t<T>;
        T prev = {};
        auto bitmapped_pairs = underlying_t{0};
        template for (constexpr auto m : std::define_static_array(enumerators_of(^^T))){ 
            T const current = [:m:];
            if (std::to_underlying(prev) * 2 == std::to_underlying(current)){
                bitmapped_pairs++;
            }
        }
        auto threshold = round_to<underlying_t>(0.75 * enum_count<T>());
        return (bitmapped_pairs >= threshold);
    }

    template<typename T>
    consteval auto enum_max() -> std::underlying_type_t<T>{
        using underlying_t = std::underlying_type_t<T>;
        auto max_val = underlying_t{0};
        // check the first 5 elements
        template for (constexpr auto m : std::define_static_array(enumerators_of(^^T))){ 
            constexpr T current = [:m:];
            max_val = std::max(max_val, std::to_underlying(current));
        }
        // if the max value exceeds the count by a factor of 2 or greater,
        return max_val;
    }
#endif
};

// NOTE: 
// EnumMap stores a M (mapped_type) for a contiguous enum K (key_type) 
template<typename K, typename M, size_t tSize = detail::enum_max<K>()>
    requires is_enum<K>
struct EnumMap{
//    static_assert(!detail::enum_is_bitmap<K>(), "You probably shouldnt be using this on a bitmapped enum, you are wasting a lot of space.");
    struct Entry{
        K key;
        M val;
    };
    static constexpr auto capacity = cast<std::size_t>(tSize);
    using key_type = K;
    using mapped_type = M;

    std::array<mapped_type,capacity> data{};



    constexpr EnumMap() = default;
    constexpr EnumMap(std::initializer_list<Entry> entries) noexcept {
        for (const auto& [k,v] : entries){
            data[std::to_underlying(k)] = v;
        }
    }

    constexpr decltype(auto) operator[](this auto& self, key_type key)noexcept{
        return self.data[std::to_underlying(key)];
    }
    constexpr decltype(auto) at(this auto& self, key_type key) noexcept{
        return self.data.at(std::to_underlying(key));
    }
    constexpr auto begin(this auto& self)noexcept{
        return self.data.begin();
    }
    constexpr auto end(this auto& self)noexcept{
        return self.data.end();
    }
    constexpr auto size() const noexcept {
        return capacity;
    }
    constexpr auto size_bytes() const noexcept {
        return capacity * sizeof(mapped_type);
    }

};
