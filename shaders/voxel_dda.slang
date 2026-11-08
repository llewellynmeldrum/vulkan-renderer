#include "../include/shaders/glm_types.slangh"
#include "../include/shaders/color_helpers.slangh"
#include "../include/shared/cpp_slang_shared.hpp"

struct VSInput{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv_pos;
};

struct VSOutput{
    glm::vec4 sv_position : SV_Position;
    glm::vec3 fragColor;
    glm::vec2 fragTxCoord;
};
// inputs should be a full screen quad?
[shader("vertex")]
VSOutput vertMain(VSInput in){
    return {};
}

[shader("fragment")]
glm::vec4 fragMain(VSOutput in): SV_Target{
    return glm::vec4(in.fragColor, 1.0);
}
