#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus

#include <print>
#include <cpptrace/basic.hpp>
#include <cpptrace/utils.hpp>
#include <cstdlib>
#include <source_location>
#include <libassert/assert.hpp>

#include "style.hpp"
#include "cppslop.hpp"
#include "timer.hpp"
#include "globals.hpp"
inline constexpr void LOG_STR(
    std::string_view title_style, 
    std::string_view title, 
    std::string_view body_style=""sv, 
    std::string_view body=""sv,
    std::source_location l= std::source_location::current()
){
    std::println(
        "{}{}{}: {}",
        style::with(title_style, title),
        global::line_no ? std::format("->[{}:{}]", l.file_name(),l.line()) : "",
        global::timer ? std::format("@[{:.2f}]", timer::to_seconds(timer::since_epoch())) : "",
        style::with(body_style, body)
   );
}
template <class... _Args>
inline constexpr void LOG_EXIT(int code) {
    auto sty = style::fg_green();
    if (code != EXIT_SUCCESS){
        sty = style::fg_red() + style::underline();
    }
    LOG_STR(
        sty,
        "EXIT",
        sty,
        std::format("Exiting with error code {}",code)
    );
    if (code!= EXIT_SUCCESS){
        cpptrace::generate_trace(1).print(); 
    }
    std::exit(code);
}

template <class... _Args>
inline constexpr void LOG_ERROR(std::format_string<_Args...> a_fmt, _Args&&... a_args) {
    LOG_STR(
        style::fg_br_red(),
        "ERROR"sv,
        style::fg_red(),
        std::format(a_fmt,std::forward<_Args>(a_args)...)
    );
}
template <class... _Args>
inline constexpr void LOG_FATAL(std::format_string<_Args...> a_fmt, _Args&&... a_args) {
    LOG_STR(
        style::fg_br_red(),
        "FATAL"sv,
        style::fg_red(),
        std::format(a_fmt,std::forward<_Args>(a_args)...)
    );
    LOG_EXIT(EXIT_FAILURE);
}
template <class... _Args>
inline constexpr void LOG_INFO(std::format_string<_Args...> a_fmt, _Args&&... a_args) {
    LOG_STR(
        style::fg_blue(),
        "INFO"sv,
        style::fg_br_blue(),
        std::format(a_fmt,std::forward<_Args>(a_args)...)
    );
}
template <class... _Args>
inline constexpr void LOG_WARN(std::format_string<_Args...> a_fmt, _Args&&... a_args) {
    LOG_STR(
        style::fg_yellow(),
        "WARN"sv,
        style::fg_br_yellow(),
        std::format(a_fmt,std::forward<_Args>(a_args)...)
    );
}
#define LOG_DBG_EXPR(expr) LOG_DBG("{} = {}", #expr, expr);
template <class... _Args>
inline constexpr void LOG_DBG(std::format_string<_Args...> a_fmt, _Args&&... a_args) {
    LOG_STR(
        style::fg_rgb(140,0,190),
        "<DBG>"sv,
        style::fg_rgb(190,0,255),
        std::format(a_fmt,std::forward<_Args>(a_args)...)
    );
}
