#pragma once 

#include "common.hpp"
#if CPP26
#include <meta>
#endif
#include <libassert/assert.hpp>

#include "primitive_types.hpp"
#include "zstring_view.hpp"
#include "concept_helpers.hpp"
#include "cppslop.hpp"


#define STABLE_NAME_LIST                    \
    X( i8, "i8")                            \
    X(i16, "i16")                           \
    X(i32, "i32")                           \
    X(i64, "i64")                           \
    X( u8, "u8")                            \
    X(u16, "u16")                           \
    X(u32, "u32")                           \
    X(u64, "u64")                           \
    X(bool, "bool")                         \
    X(f16, "f16")                           \
    X(f32, "f32")                           \
    X(f64, "f64")                           \
    X(std::string, "std::string")           \
    X(std::string_view, "std::string_view")

template<typename T>
inline consteval auto stable_type_name_zstr(){
    #define X(U, NAME) if (same_as_nocvref<T,U>) {return zstring_view{NAME};}
    STABLE_NAME_LIST
    #undef X
#if CPP26
    return zstring_view(std::define_static_string(display_string_of(remove_cv(^^T))));
#endif
}
template<typename T>
inline consteval auto stable_type_name(){
    #define X(U, NAME) if (same_as_nocvref<T,U>) {return NAME ##sv;}
    STABLE_NAME_LIST
    #undef X
#if CPP26
    return display_string_of(remove_cv(^^T));
#endif
}

// NOTE: since std::meta::identifier_of is not stable, we use these
#if CPP26
inline consteval auto stable_type_name_meta(std::meta::info m){
    using namespace std::meta;
    if (!std::meta::is_type(m)){
        throw std::meta::exception("stable_type_name_meta must be provided with a value that represents a type.",m);
    }
    #define X(T, NAME) if (remove_cv(m) == dealias(^^T)) {return NAME ##sv;}
    STABLE_NAME_LIST
    #undef X
    return display_string_of(remove_cv(m));
}
inline consteval auto stable_type_name_meta_zstr(std::meta::info m) ->zstring_view{
    if (!std::meta::is_type(m)){
        throw std::meta::exception("stable_type_name_meta_zstr must be provided with a value that represents a type.",m);
    }
    using namespace std::meta;
    #define X(T, NAME) if (dealias(remove_cv(m)) == dealias(^^T)) {return zstring_view(NAME);}
    STABLE_NAME_LIST
    #undef X
    return zstring_view(std::define_static_string(display_string_of(remove_cv(m))));
}
#endif 
