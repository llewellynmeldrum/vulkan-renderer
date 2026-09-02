#include "glsl_types.slangh"

struct UBO{
    mat4 model;
    mat4 view;
    mat4 proj;
};

struct VSInput{
    vec3 pos;
    vec3 color;
    vec2 texCoord;
};

struct VSOutput{
    vec4 sv_position : SV_Position;
    vec3 fragColor;
    vec2 fragTxCoord;
};
ConstantBuffer<UBO> ubo;

Sampler2D texture;
[shader("vertex")]
VSOutput vertMain(VSInput in){
    VSOutput output;
    output.sv_position = 
    mul(ubo.proj,
        mul(ubo.view,
            mul(ubo.model,
                vec4(in.pos,1.0)
            )
        )
    )
    ;
    output.fragColor = in.color;
    output.fragTxCoord = in.texCoord;
    return output;
}

[shader("fragment")]
vec4 fragMain(VSOutput in): SV_Target{
    return vec4(in.fragColor,1.0f);
}
