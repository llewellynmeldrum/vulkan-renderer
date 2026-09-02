#pragma once 

#include "types.hpp"
#include "vertex.hpp"
struct CpuMesh{
    static constexpr auto IndicesPerQuad{6};
    static constexpr auto VerticesPerQuad{4};
    using IndexType = u32;
    using VertexType = Vertex;
    std::vector<VertexType> vertices;
    std::vector<IndexType> indices;
    u32 m_quad_count{0};
    void add_quad(std::array<glm::vec3,4> vtx_positions, std::array<glm::vec3,4> vtx_colors = {});
};
