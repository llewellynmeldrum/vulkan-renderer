#pragma once 
#include "glm_types.hpp"
#include <array>
#include <span>
#include <ranges>
template<typename V>
struct QuadVertices{
    union{
        struct {
            V m_bot_left{};
            V m_bot_right{};
            V m_top_right{};
            V m_top_left{};
        };
        std::array<V, 4> m_all;
    };

    QuadVertices() = default;
    QuadVertices( V bot_left, V bot_right, V top_right, V top_left)
        :m_bot_left (bot_left)
        ,m_bot_right(bot_right)
        ,m_top_right(top_right)
        ,m_top_left (top_left)
    {}
    QuadVertices(std::span<const V,4> span) {
        std::ranges::copy(span,m_all);
    }

    constexpr auto operator[](this auto& self, std::size_t idx) -> decltype(auto){return self.m_all[idx];}
    constexpr auto span(this auto& self) -> decltype(auto) { return std::span(self.m_all); }
    static constexpr auto size() -> std::size_t{ return 4; }
};
