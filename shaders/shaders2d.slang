#include "glsl_types.slangh"
#include "color_helpers.slangh"
struct Transform2D{
    vec2 scale;
    vec2 translate;
//    u8 z;
};

[vk::push_constant]
Transform2D transform;

static u32 ShapeID_CircleMask = 0x0;
struct VSInput{
    vec2 pos;
    vec4 color;
    vec2 texCoord;
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
    output.fragTxCoord = in.texCoord;
    output.shapeID = in.shapeID;
    return output;
}

f32 sdCircle (vec2 pos, vec2 center, f32 radius){
    return length(pos - center) - radius;
}
[shader("fragment")]
vec4 fragMain(VSOutput in): SV_Target{
//    u32 is_circle_mask = in.shapeID & ShapeID_CircleMask;
    // non circle objects (quads) draw without any contribution from center_dist
    f32 radius_ndc = 0.5;
    f32 c = sdCircle(in.fragTxCoord, {0.5,0.5}, radius_ndc);
    // -1 inside circle
    // 0 on edge
    // +1 outside circle
    // we want some scalar which remaps this range, st opacity is 1 inside 
    // c  | opacity
    // -1 |  1      (inside circle)
    //  0 |  ?      (on edge)
    // +1 |  0      (outside circle)
    f32 opacity = 1.0 - smoothstep(0,0.01, c);
    return in.fragColor * opacity;

}

