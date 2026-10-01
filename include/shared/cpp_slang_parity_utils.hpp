#pragma once 
// NOTE: this file must work in both c++ and slang. 
// Including it provides utilities to write code which can be compiled by slangc or a regular c++ compiler.


#define SIZE_BYTES(x)   (sizeof(x))
#define SIZE_BITS(x)    ((sizeof(x)) * (8))

// NOTE: Compatability includes
#if defined(__cplusplus)
    #include <array>
    #include "glm_types.hpp"
#else
    #include "shaders/glm_types.slangh"
#endif 

// NOTE: Compatability definitions
#if defined(__cplusplus)
    #define TYPEDEF using
    #define IF_CPP(...) __VA_ARGS__
    #define IF_SLANG(...) 
    #define IF_CPP_ELSE(EXPR_IFF_CPP, EXPR_IFF_SLANG) EXPR_IFF_CPP
    #define CAST(TYPE, VAR) static_cast<TYPE>(VAR)
    #define CONSTEXPR_VAR constexpr
    #define SCOPED_ENUM enum struct
    #define DECL_ALIGNED(T) IF_CPP(alignas(SIZE_BYTES(T))) T
#else
    #define TYPEDEF typealias
    #define IF_CPP(...)
    #define IF_SLANG(...) __VA_ARGS__
    #define IF_CPP_ELSE(EXPR_IFF_CPP, EXPR_IFF_SLANG) EXPR_IFF_SLANG
    #define CAST(TYPE, VAR) (TYPE)(VAR)
    #define CONSTEXPR_VAR const
    #define SCOPED_ENUM enum 
    #define DECL_ALIGNED(T) T
#endif 


// NOTE: Array helper, in order to be able to define arrays in slang like c++ std::arrays.
// Usage: array<T, N> meow({...});
IF_CPP(
    template<typename T, std::size_t size> using array = std::array<T, size>;
)
IF_SLANG(
    struct array<T, let N : int> { T arrayContent[N]; };
)



