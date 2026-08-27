#pragma once 

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include "vk_types.hpp"
#include "vertex_helpers.hpp"

struct Vertex{
    glm::vec2 pos;
    glm::vec3 color;

    static constexpr auto binding_description(){
        return vk::VertexInputBindingDescription{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = vk::VertexInputRate::eVertex
        };
    }
    static constexpr auto attribute_descriptions(){
        return std::array{
            MAKE_VATTR_DESC(0, Vertex, pos),
            MAKE_VATTR_DESC(1, Vertex, color),
        };
    }
};

