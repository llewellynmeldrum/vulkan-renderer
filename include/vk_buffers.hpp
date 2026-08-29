#pragma once 

#include "glm/ext/matrix_float4x4.hpp"

struct UniformBufferObject{
    alignas(16) glm::mat4x4 model;
    alignas(16) glm::mat4x4 view;
    alignas(16) glm::mat4x4 proj;
};
