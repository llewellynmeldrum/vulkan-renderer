#include "cpu_mesh.hpp"
#include "vertex_raw_data.hpp"
#include "format_specs.hpp"
// NOTE:
// Expects vertices to be in 
void CpuMesh::add_quad(std::array<glm::vec3,4> vtx_positions, std::array<glm::vec3,4> vtx_colors ){
    auto index_offset = vertices.size();

//    LOG_INFO("quad verts: {}, {}, {}, {}",vtx_positions[0],vtx_positions[1],vtx_positions[2],vtx_positions[3]);
    for (int i = 0; i<CpuMesh::VerticesPerQuad; i++){
        vertices.emplace_back(
            vtx_positions[i],
            vtx_colors[i],
            vtx_raw_data::ccw_quad_verts[i].texCoord
        );
    }
    for (int i = 0; i<CpuMesh::IndicesPerQuad; i++){
        auto index = index_offset + vtx_raw_data::ccw_quad_indices[i];
        ASSERT(index < vertices.size());
        indices.emplace_back(index);
    }
    m_quad_count++;
}
