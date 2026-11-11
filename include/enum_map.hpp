#pragma once
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

namespace detail{
    template<typename T>
    consteval auto enum_max() -> std::size_t{
        auto max_val = 0uz;
#if CPP26
        template for (constexpr auto m : std::define_static_array(enumerators_of(^^T))){
            max_val = std::max(max_val, static_cast<std::size_t>([:m:]));
        }
#endif
        return max_val;
    }
};
template<typename K, typename M, size_t size = detail::enum_max<K>()>
    requires is_enum<K>
struct EnumMap{
    struct Entry{
        K key;
        M val;
    };
    using key_type = K;
    using mapped_type = M;

    static constexpr size_t capacity = size;


    constexpr EnumMap() = default;
    constexpr EnumMap(std::initializer_list<Entry> entries) noexcept {
        for (const auto& [k,v] : entries){
            data[std::to_underlying(k)] = v;
        }
    }

    constexpr decltype(auto) operator[](this auto& self, key_type key)noexcept{
        return self.data[std::to_underlying(key)];
    }
    constexpr decltype(auto) at(this auto& self, key_type key)noexcept{
        return self.data.at(std::to_underlying(key));
    }
    constexpr auto begin(this auto& self)noexcept{
        return self.data.begin();
    }
    constexpr auto end(this auto& self)noexcept{
        return self.data.end();
    }

    std::array<mapped_type,capacity> data{};
};
