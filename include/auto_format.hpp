#pragma once 
// USAGE:
// Annotate your structs declaration with the refl::auto_format.
// Alternatively, if you want something clangd23 friendly, use the ANNO_AUTO_FMT macro.

#include <ranges>
#include <format>

#include "meta_shorthand.hpp"
#include "common.hpp"
#include "meta_helpers.hpp"
#include "annotation_helpers.hpp"
#include "static_styling.hpp"
//#include "enum_to_string.hpp"

#if !CLANGD_PARSE
#define AUTO_FORMAT [[=refl::auto_format]]
#define AUTO_HASH [[=refl::auto_format]]
#else
#define AUTO_FORMAT 
#define AUTO_HASH
#endif

// NOTE: Enums are automatically formattable. 
#if CPP26
template<typename T> requires is_enum<T>
struct std::formatter<T>{
    constexpr auto parse(std::format_parse_context& ctx) {return ctx.begin();}

    constexpr auto format(const T& obj, auto& ctx) const noexcept{
        std::string  name = "";
        using namespace std::meta;
        template for (constexpr auto m: std::define_static_array(enumerators_of(^^T))){
            if ([:m:] == obj){
                name = std::meta::identifier_of(m);
            }
        }
        return std::format_to(ctx.out(), "{}",name);
    }
};
namespace refl{

struct auto_format_tag{};
struct auto_hash_tag{};
inline constexpr auto auto_format = auto_format_tag{};
inline constexpr auto auto_hash = auto_hash_tag{};


consteval bool verify_auto_format_tag(std::meta::info m){
    if (std::meta::is_enum_type(m)){
        throw std::meta::exception(
            "Auto formatter attribute should not be applied to enums! "
            "They are already formattable by default by virtue of the formatter at the top of this file ("+std::string(__FILE__)+"). "
            "HINT: Remove the AUTO_FORMATTER attribute from "+std::string(std::meta::display_string_of(m))+"'s declaration."sv
            , ^^verify_auto_format_tag);
    }
    return true;
}
template<typename T>
concept auto_formattable = has_annotation<refl::auto_format_tag>(^^T) && verify_auto_format_tag(^^T);

template<typename T>
concept auto_hashable = has_annotation<refl::auto_hash_tag>(^^T);


struct auto_formatter{
    struct format_opts{
        bool newlines_and_indents{false};
        bool print_types{false};
        bool include_static{false};
    }opts;

    // options: 
    // {:t} subfields show their type names
    // {:s} include static members
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        while (it != ctx.end() && *it != '}'){
            switch (*it){
                case 'n': opts.newlines_and_indents= true; break;
                case 't': opts.print_types = true; break;
                case 's': opts.include_static= true; break;
                default: throw std::format_error("auto_formatter: Unknown option");
            }
            it++;
        }
        return it;
        return ctx.begin(); 
    }
    //format()
    static constexpr auto apply_indent(auto& ctx, size_t indent)noexcept{
        auto out = std::format_to(ctx.out(),"");
        for (size_t i = 0; i<indent; i++){
            out = std::format_to(ctx.out(), "    ");
        }
        return out;
    };
    template<typename T>
    constexpr auto format_natively(
        T const& obj,
        std::string_view identifier,
        auto& ctx,
        format_opts const& opts,
        size_t indent
    )const noexcept{
        auto res = std::format_to(ctx.out(),"");
        if (opts.print_types){
            res = std::format_to(
                ctx.out(), ".{} : {} = {}, ",
                identifier,
                std::meta::display_string_of(^^T),
                obj
            );
        } else{
            res = std::format_to(
                ctx.out(), ".{} = {},",
                identifier,
                obj
            );
        }
        return res;
    }

    template<typename T>
    static constexpr auto format_field_name(
        T const& obj,
        std::string_view identifier,
        auto& ctx,
        format_opts const& opts
    ) noexcept{
        if (opts.print_types){
            return std::format_to(
                ctx.out(), 
                "{}{} : {} {{ {}",
                identifier=="" ? "" : ".",
                identifier,
                std::meta::display_string_of(^^T),
                opts.newlines_and_indents ? "\n" : ""
            );
        }else{
            return std::format_to(
                ctx.out(),
                "{}{} {{ {}",
                identifier == "" ? "" : ".",
                identifier,
                opts.newlines_and_indents ? "\n" : ""
            );
        }
    }
    auto newline(auto& ctx) const{
         return std::format_to(ctx.out(), "{}", opts.newlines_and_indents ? "\n" : "");
    }
    template<typename T>
    auto recurse_format(
        T const& obj,
        std::string_view identifier,
        auto& ctx,
        format_opts const& opts,
        size_t indent,
        bool is_static
    )const noexcept{
        auto out = std::format_to(ctx.out(), "");
        // base case: formattable natively (i.e not one of ours)
        if (opts.newlines_and_indents) out = apply_indent(ctx, indent);
        if constexpr (auto_formattable<T>){
            if (is_static){
                out = std::format_to(ctx.out(), "(static)");
            }
            out = format_field_name(obj, identifier, ctx, opts);
            if (opts.include_static){
                template for (constexpr auto m: static_members_array<T>()){
                    using U = decltype(obj.[:m:]);
                    out = recurse_format<U>(obj.[:m:], std::meta::identifier_of(m), ctx, opts, indent+1,true);
                    out = newline(ctx);
                }
            }
            template for (constexpr auto m: nonstatic_members_array<T>()){
                using U = decltype(obj.[:m:]);
                out = recurse_format<U>(obj.[:m:], std::meta::identifier_of(m), ctx, opts, indent+1,false);
                out = newline(ctx);
            }
            if (opts.newlines_and_indents) out = apply_indent(ctx, indent);
            out = std::format_to(ctx.out(), "}}");
        } else if constexpr (std::formattable<T,char>){
            out = format_natively(obj, identifier, ctx,opts, indent+1);
        }else{
            static_assert(false, "T=" + std::string(stable_type_name<T>()) + " is not formattable natively, and does not have the auto format trait.");
        }
        return out;
    }


    template<typename T>
    constexpr auto format(const T& obj, auto& ctx) const noexcept{
        return recurse_format(obj,"",ctx,opts,0,false);
    }

};
} // NOTE: namespace refl


template<typename T> requires refl::auto_hashable<T> 
struct std::hash<T>{
    constexpr auto operator()(T const& v) const noexcept -> std::size_t{
        if constexpr (!std::meta::is_complete_type(^^T)){
            throw std::meta::exception("auto hasher : [with T="+std::string(std::meta::display_string_of(^^T))+"] T must be a complete type."sv,^^T);
        }else if constexpr (std::is_class_v<T>){
            template for(constexpr auto m: nonstatic_members_array<T>){
                using Mt = typename [:type_of(m):];
                return std::hash<Mt>{}(v.[:m:]);
            }
        }else{
            return std::hash<T>{}(v);
        }
    };
};

template<typename T> requires refl::auto_formattable<T> 
struct std::formatter<T> : refl::auto_formatter { };
#endif // CPP26
