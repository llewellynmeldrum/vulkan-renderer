
struct VSInput{
    float2 pos;
    float3 color;
};

struct VSOutput{
    float4 sv_position : SV_Position;
    float3 color;
};


[shader("vertex")]
VSOutput vertMain(VSInput in){
    VSOutput output;
    output.sv_position = float4(in.pos,0.0,1.0);
    output.color = in.color;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput in): SV_Target{
    return float4(in.color,1.0);
}
