#include "glsl_types.slangh"
#include "color_helpers.slangh"
#include "../include/cpp_slang_shared.hpp"
struct Transform2D{
    vec2 scale;
    vec2 translate;
//    u8 z;
};

[vk::push_constant]
Transform2D transform;


struct VSInput{
    vec2 pos;
    vec4 color;
    vec2 uv_pos;
    vec2 shape_center_pos;
    u32 shapeID;
};

struct VSOutput{
    vec4 sv_position : SV_Position;
    vec4 fragColor;
    vec2 fragTxCoord;
    nointerpolation vec2 center_pos : SOME_VALUE;
    u32 shapeID;
};

// TODO: fix this
[shader("vertex")]
VSOutput vertMain(VSInput in){
    VSOutput output;
    vec2 pos = in.pos * transform.scale + transform.translate;
    vec2 center_pos = in.shape_center_pos * transform.scale + transform.translate;
    output.center_pos = center_pos; 
    output.sv_position = vec4(pos, 1.0,1.0);
    output.fragColor = in.color;
    output.fragTxCoord = in.uv_pos;
    output.shapeID = in.shapeID;
    return output;
}

f32 sdCircle (vec2 pos, vec2 center, f32 radius){
    // -1 inside circle
    // 0 on edge
    // +1 outside circle
    return length(pos - center) - radius;
}
[shader("fragment")]
vec4 fragMain(VSOutput in): SV_Target{
//    u32 is_circle_mask = in.shapeID & ShapeID_CircleMask;
    // non circle objects (quads) draw without any contribution from center_dist
    f32 radius_ndc = 0.5 / circle_edge_correction_scale;
    f32 c = sdCircle(in.fragTxCoord, {0.5,0.5}, radius_ndc);
    f32 opacity = 1.0 - smoothstep(0,0.01, c);
    i32 not_circle_factor = 1 - shapeID_Circle.check(in.shapeID);
    // 1 if circle, else 0 
    // if circle_factor is 1, opacity = 1
    // if circle_factor = 0, opacity = opacity
    // if not a circle, opacity = 1
    opacity = max(not_circle_factor, opacity);
    return in.fragColor * opacity;

}

