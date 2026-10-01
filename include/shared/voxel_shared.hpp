#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
// NOTE: this file must work in both c++ and slang. 

#include "shared/cpp_slang_parity_utils.hpp"

#define MAKE_RGB(r,g,b) IF_CPP(glm::) vec3{r/255.0,g/255.0,b/255.0}
#define MAKE_RGBA(r,g,b,a) IF_CPP(glm::) vec4{r/255.0,g/255.0,b/255.0, a/255.0}

SCOPED_ENUM VoxelMaterial : u8{
    eAir        = 0,
    eSand       = 1,
    eStone      = 2,
};

IF_CPP_ELSE(inline constexpr, static global) 
array<IF_CPP(glm::)vec3, 3> test({
    MAKE_RGBA(255,255,255,0), // air
    MAKE_RGBA(240,220, 40,255), // sand
    MAKE_RGBA(120,120,120,255) //stone 
});

#undef MAKE_RGB
#undef MAKE_RGBA
