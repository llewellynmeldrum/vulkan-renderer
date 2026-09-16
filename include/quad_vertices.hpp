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

    template<typename Range> requires std::ranges::input_range<Range>
    QuadVertices(Range input_range) {
        if constexpr (std::ranges::sized_range<Range>) {
            if (std::ranges::size(input_range) != 4) {
                throw std::invalid_argument("Range must contain exactly 4 elements.");
            }
        }
        std::ranges::copy(input_range,m_all);
    }

    constexpr auto operator[](std::size_t idx) -> decltype(auto){return m_all[idx];}
    constexpr auto span() -> auto{ return std::span(m_all); }
    static constexpr auto size() -> std::size_t{ return 4; }
};
