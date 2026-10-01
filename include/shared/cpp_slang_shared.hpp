#pragma once 

#include "shared/cpp_slang_parity_utils.hpp"


struct UBO{
//    DECL_ALIGNED(glm::mat4) model;
    DECL_ALIGNED(glm::mat4) view;
    DECL_ALIGNED(glm::mat4) proj;
};

#include "shared/primitive_types.hpp"


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
static CONSTEXPR_VAR BitMask shapeID_TextGlyph = BitMask(2);

static CONSTEXPR_VAR f32 circle_edge_correction_scale = 1.2f;


namespace PushConstants{
    struct Transform2D{
        DECL_ALIGNED(glm::vec2) scale;
        DECL_ALIGNED(glm::vec2) translate;
    };
    struct ModelMatrix{
        DECL_ALIGNED(glm::mat4) model;
    };
}

