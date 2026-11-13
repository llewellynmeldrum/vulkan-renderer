#pragma once

#include <format>
#include <type_traits>
#include "glm/detail/qualifier.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "primitive_types.hpp"

template<glm::length_t L, typename T, glm::qualifier Q>
struct std::formatter<glm::vec<L,T,Q>>{
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }
    constexpr auto format(const glm::vec<L,T,Q>& obj, auto& ctx) const noexcept{
        std::format_to(ctx.out(),"{}", "{{");
        auto first = true;
        for (const auto& v : std::span(glm::value_ptr(obj),L)){
            if (!first){
                std::format_to(ctx.out(),", ");
            }
            std::format_to(ctx.out(),"{}", v);
            first = false;
        }
        std::format_to(ctx.out(),"{}", "}}");
    }
};

template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct std::formatter<glm::mat<C,R,T,Q>>{

	constexpr auto parse(std::format_parse_context& ctx){return ctx.begin();}
	auto format(const glm::mat4& val, auto& ctx)const {
        std::string res{};
        res.append("\n");
        for (i64 row = 0; row < R; row++) {
            res.append("| ");
            for (i64 col = 0; col < C; col++) {
                res.append(std::format("{: 3.1f}", val[col][row]));
                if (col != C - 1) {
                    res.append(" ");
                }
            }
            res.append(" |");
            if (row != R - 1)
                res.append("\n");
        }
        return format_to(ctx.out(), "{}",res);
    }
};
