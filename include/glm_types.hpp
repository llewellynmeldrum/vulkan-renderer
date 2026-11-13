#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "primitive_types.hpp"

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
