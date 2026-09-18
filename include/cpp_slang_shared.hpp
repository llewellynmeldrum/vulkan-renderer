#ifndef CPP_SLANG_SHARED_HPP
#define CPP_SLANG_SHARED_HPP

// NOTE:
// Header which can be included in both .cpp and .slang files.
// Some features used may be exclusive to whatever slang im using (whatever the default is on 2026.13.1-1-g84792eb15)
//


// NOTE: Includes
#if defined(__cplusplus)
    #include <glm_types.hpp>
#else
    #include "../shaders/color_helpers.slangh"

#endif 

// NOTE: Compatability definitions
#if defined(__cplusplus)
    #define TYPEDEF using
    #define IF_CPP(...) __VA_ARGS__
    #define IF_CPP_ELSE(EXPR_IFF_CPP, EXPR_IFF_SLANG) EXPR_IFF_CPP
    #define CAST(TYPE, VAR) static_cast<TYPE>(VAR)
    #define CONSTEXPR_VAR constexpr
#else
    #define TYPEDEF typealias
    #define IF_CPP(...)
    #define IF_CPP_ELSE(EXPR_IFF_CPP, EXPR_IFF_SLANG) EXPR_IFF_SLANG
    #define CAST(TYPE, VAR) (TYPE)(VAR)
    #define CONSTEXPR_VAR const
#endif 



struct UBO{
    IF_CPP(alignas(16) glm::) mat4x4 model;
    IF_CPP(alignas(16) glm::) mat4x4 view;
    IF_CPP(alignas(16) glm::) mat4x4 proj;
};

IF_CPP(using float16_t = _Float16;)
IF_CPP(using float32_t = float;)
IF_CPP(using float64_t = double;)

TYPEDEF u8  = uint8_t;
TYPEDEF u16 = uint16_t;
TYPEDEF u32 = uint32_t;
TYPEDEF u64 = uint64_t;
TYPEDEF i8  =   int8_t;
TYPEDEF i16 =  int16_t;
TYPEDEF i32 =  int32_t;
TYPEDEF i64 =  int64_t;

TYPEDEF f16 = float16_t;
TYPEDEF f32 = float32_t;
TYPEDEF f64 = float64_t;

struct BitMask{
    u32 offset;
    bool check(u32 v) IF_CPP(const noexcept) {
        return CAST(bool,(v >> offset) & 1);
    }
    u32 get() IF_CPP(const noexcept) {
        return 1U << offset;
    }
};
static CONSTEXPR_VAR BitMask shapeID_Quad = BitMask(0);
static CONSTEXPR_VAR BitMask shapeID_Circle = BitMask(1);
static CONSTEXPR_VAR f32 circle_edge_correction_scale = 1.2f;

#endif // CPP_SLANG_SHARED_HPP
