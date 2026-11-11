#pragma once 
#include <string>
#include <ranges>
#include <format>

#include "span_wrapper.hpp"

struct zstring_view{
    // NOTE: No std::string_view ctor on purpose.
    using char_t = char;
    const_dyn_span<char_t> m_span;

    constexpr zstring_view(const char* cstr): m_span(cstr,std::char_traits<char_t>::length(cstr)) {}
    constexpr zstring_view(const char* cstr, size_t len): m_span(cstr,len) {}
    constexpr zstring_view(std::string const& str ): m_span(str.c_str(),str.length()) {}
    constexpr explicit operator std::string_view() const {return std::string_view{m_span};}

    constexpr auto clone () const -> std::string{return std::string(std::from_range, m_span);}
    constexpr auto cstr() const -> const char*{return m_span.data();}
    constexpr auto empty() const -> bool {return m_span.empty();}
    constexpr auto size() const -> size_t {return m_span.size();}
    constexpr auto length() const -> size_t {return m_span.size();}
    auto begin(this auto&& m) {return m.m_span.begin();}
    auto end(this auto&& m) {return m.m_span.end();}
};
template<> 
struct std::formatter<zstring_view>{
    bool bytes{false};
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        while (it != ctx.end() && *it != '}'){
            switch (*it){
                case 'b': bytes=true; break;
                default: throw std::format_error("auto_formatter: Unknown option");
            }
            it++;
        }
        return it;
    }

    // TODO: REMOVE THE ALLOCATION
    constexpr auto format(const zstring_view& obj, auto& ctx) const noexcept{
        auto out = ctx.out();
        if (bytes){
            for (const char c : obj){
                out = std::format_to(ctx.out(), "{:02X} ", c);
            }
            out = std::format_to(ctx.out(), "\n");
            for (const char c : obj){
                out = std::format_to(ctx.out(), "{{}},", c);
            }
        }
        return std::format_to(ctx.out(), "{}",obj.clone());
    }
};
template<> 
struct std::hash<zstring_view>{
    // TODO: REMOVE THE ALLOCATION
    constexpr auto operator()(zstring_view const& v) const noexcept{
        return std::hash<std::string>{}(v.clone());
    }
};


namespace zstring_view_literals {
#pragma GCC diagnostic push
#if CLANGD_PARSE
    #pragma GCC diagnostic ignored "-Wreserved-user-defined-literal"
    #pragma GCC diagnostic ignored "-Wuser-defined-literals"
#else
    #pragma GCC diagnostic ignored "-Wliteral-suffix"
#endif
    inline constexpr zstring_view
    operator""_zv(const char* cstr, size_t len) noexcept {
        return zstring_view{cstr, len}; 
    }
#pragma GCC diagnostic pop
static constexpr auto zstr = "hi"_zv;
} // namespace zstring_literals

