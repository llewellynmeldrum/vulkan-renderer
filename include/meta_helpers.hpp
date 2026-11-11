#pragma once 

#include <string_view>
#include <type_traits>
#include "concept_helpers.hpp"
#include "meta_shorthand.hpp"
#if CPP26
#include "primitive_types.hpp"
#include "cppslop.hpp"
#include "stable_type_name.hpp"
using namespace std::string_view_literals;
#define COMPTIME_STR(s) (std::define_static_string(s))
#define COMPTIME_ARR(a) (std::define_static_array(a))

template<typename T> 
inline consteval auto comptime_arr(T const& v) -> decltype(auto){
    return std::define_static_array(v);
}
template<typename T> 
inline consteval auto comptime_str(T const& v) -> decltype(auto){
    return std::define_static_string(v);
}

template<typename T>
inline consteval auto is_annotation_of(std::meta::info m) -> bool{
    using namespace std::meta;
    auto type = dealias(remove_cvref(type_of(m)));
    return type == ^^T;
}

template<typename T>
inline constexpr auto nonstatic_members_array(){
    if constexpr (!std::is_class_v<T>){
        throw std::meta::exception("nonstatic_members_array() : [with T="+std::string(std::meta::display_string_of(^^T))+"] T must be a class type."sv,^^T);
    }else if constexpr (!std::meta::is_complete_type(^^T)){
        throw std::meta::exception("nonstatic_members_array() : [with T="+std::string(std::meta::display_string_of(^^T))+"] T must be a complete type."sv,^^T);
    }
    return std::define_static_array(
        std::meta::nonstatic_data_members_of(
            ^^T, std::meta::access_context::current()
        )
    );
};
template<std::meta::info m>
inline constexpr auto annotations_array(){
    return std::define_static_array(
        std::meta::annotations_of(
            m
        )
    );
};
template<std::meta::info m>
inline constexpr auto enumerators_of(){
    return std::define_static_array(
        std::meta::enumerators_of(
            m
        )
    );
};


template<typename T>
static constexpr auto static_members_array(){
    return std::define_static_array(
        std::meta::static_data_members_of(
            ^^T, std::meta::access_context::current()
        )
    );
};

struct ReflectionComment{ char const* text; };
struct FieldRename{ char const* new_name; };




template<typename T, typename Fn>
[[nodiscard]] consteval auto nonstatic_members_partitioned(Fn&& pred){
    auto out = std::vector<std::meta::info>{};
    template for (constexpr auto m: nonstatic_members_array<T>()){
        if (pred(m)){
            out.push_back(m);
        }
    }
    template for (constexpr auto m: nonstatic_members_array<T>()){
        if (!pred(m)){
            out.push_back(m);
        }
    }
    return std::define_static_array(out);
}

template<typename T, typename Fn>
[[nodiscard]] consteval auto nonstatic_members_if(Fn&& pred){
    auto out = std::vector<std::meta::info>{};
    template for (constexpr auto m: nonstatic_members_array<T>()){
        if (pred(m)){
            out.push_back(m);
        }
    }
    return std::define_static_array(out);
}

template<typename ...Args>
inline consteval auto satisfies_concept(meta::info r_concept, Args ...args){
    return meta::extract<bool>(meta::substitute(r_concept, {args...}));
}
//NOTE: Meta helpers namespace
namespace mh{ // (M)eta (H)elpers
    // mh::str(v) == std::string(v)
    [[nodiscard]] constexpr auto str(std::string_view s) -> std::string{ return std::string(s);}
    [[nodiscard]] constexpr auto str(char const* s)      -> std::string{ return std::string(s);}

    template<typename ...Strs> requires (std::constructible_from<std::string,Strs> && ...)
    [[nodiscard]] constexpr auto join_strings(Strs... args) -> std::string{ return (std::string(args) += ...);}

}
#endif // CPP26
