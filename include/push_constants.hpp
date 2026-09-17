#pragma once 
#include "vk_types.hpp"
#include "common_utils.hpp"
#include <cstddef>


namespace PushConstants{
struct Transform2D{
    alignas(SIZE_BYTES(glm::vec2)) glm::vec2 scale;
    alignas(SIZE_BYTES(glm::vec2)) glm::vec2 translate;
};
// TODO: Need to implement the actual push() calls. 
// Perhaps i can embed the type of the push constants into the ShaderPipelineContext, and have 
// some sort of validation there, eg the function only accepts a matching type rather than raw bytes. 
//
// Toodooloo, good luck tomorrow

struct None;




} // namespace PushConstants
