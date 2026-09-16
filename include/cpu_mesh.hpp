#pragma once 

#include "quad_vertices.hpp"
#include "types.hpp"
#include "vertex.hpp"
#include "vertex_raw_data.hpp"

template<typename tVertexType, typename tIndexType>
struct CpuMesh{
public:
    static constexpr auto IndicesPerQuad{6};
    static constexpr auto VerticesPerQuad{4};
    using IndexType = tIndexType;
    using VertexType = tVertexType;
    using QuadVerticesType = QuadVertices<VertexType>;

    void add_quad(QuadVerticesType const& in_vertices);


    std::vector<VertexType> vertices;
    std::vector<IndexType> indices;
    u32 m_quad_count{0};

};
using CpuMesh3D = CpuMesh<Vertex3D, u32>;
using CpuMesh2D = CpuMesh<Vertex2D, u32>;


template<typename tVertexType, typename tIndexType>
void CpuMesh<tVertexType, tIndexType>::add_quad(QuadVerticesType const& in_vertices){
    auto index_offset = vertices.size();

    vertices.append_range(in_vertices.span());
    for (int i = 0; i<CpuMesh::IndicesPerQuad; i++){
        auto index = index_offset + vtx_raw_data::ccw_quad_indices[i];
        ASSERT(index < vertices.size());
        indices.emplace_back(index);
    }
    m_quad_count++;
}
