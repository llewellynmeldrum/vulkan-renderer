#pragma once 
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "types.hpp"

struct vec2xz{
    vec2xz()
        : x(0.0f)
        , z(0.0f)
    {}
    vec2xz(f32 x_, f32 z_)
        : x(x_)
        , z(z_)
    {}
    union {
        glm::vec2 glmvec;
        struct {
            f32 x;
            f32 z;
        };
    };
};
