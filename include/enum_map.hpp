#pragma once 
#include <utility>
#include <array>
#include <initializer_list>
#include "common_concepts.hpp"
#include "bitmask_flags.hpp"
#include "magic_enum.hpp"
#include "magic_enum_bitmask_flags_ext.hpp"

template<typename K, typename M, size_t size = magic_enum::ext::enum_count<K>()>
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
