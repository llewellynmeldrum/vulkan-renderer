#include "glsl_types.slangh"
#include "color_helpers.slangh"
#include "../include/cpp_slang_shared.hpp"


ConstantBuffer<UBO> ubo;
Sampler2D tim_cheese;
Sampler2D _; //unused

[vk::push_constant]
PushConstants::ModelMatrix pc;


struct VSInput{
    vec3 pos;
    vec3 color;
    vec2 uv_pos;
};

struct VSOutput{
    vec4 sv_position : SV_Position;
    vec3 fragColor;
    vec2 fragTxCoord;
};

[shader("vertex")]
VSOutput vertMain(VSInput in){
    VSOutput output;
    let world_space = mul(pc.model, vec4(in.pos,1.0));
    let view_space = mul(ubo.view, world_space);
    output.sv_position = mul(ubo.proj,view_space);
//    output.sv_position = mul(ubo.proj, mul(ubo.view, mul(model, vec4(in.pos,1.0))));
    output.fragColor = in.color;
    output.fragTxCoord = in.uv_pos;
    return output;
}

[shader("fragment")]
vec4 fragMain(VSOutput in): SV_Target{
    return vec4(in.fragColor, 1.0);
}

// We slightly shift the depth (scaled by w) of the wireframe so that it appears on top.
// Simply adding a depth bias to the pipeline is not enough.
// Also note, since we have a flipped Z, we ADD to the z, which makes it closer (1.0f means close now, 0.0f means far)) 
static const float DEPTH_NUDGE = 1e-5;
[shader("vertex")]
VSOutput vertWireframe(VSInput in){
    VSOutput output = vertMain(in);
    output.sv_position.z  += DEPTH_NUDGE * output.sv_position.w;
    return output;
}

[shader("fragment")]
vec4 fragWireframe(VSOutput in): SV_Target{
//    return vec4(invert(in.fragColor), 1.0);
    return vec4(smoothstep(0.0, 1.0, invert(in.fragColor)), 1.0);
}
// NOTE: Ensure you add any new fragment or vertex functions as an entry point in the cmake command.
