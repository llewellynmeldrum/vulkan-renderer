#pragma once 

#include "vertex.hpp"
#include "vertex_raw_data.hpp"
#include "vk_managed_buffers.hpp"
//#include "vk_traits.hpp")
template<typename tVertexType, typename tIndexType=u32>
struct GpuMesh{
    using IndexType = tIndexType;
    using VertexType = tVertexType;
    GpuMesh() = default;
    GpuMesh(const GpuMesh &) = delete;  GpuMesh &operator=(const GpuMesh &) = delete ;
    GpuMesh(GpuMesh &&) = default;      GpuMesh &operator=(GpuMesh &&) = default;

    VmaAllocator m_allocator{nullptr};
    AllocatedBuffer m_indices{nullptr};
    AllocatedBuffer m_vertices{nullptr};
    u32 m_index_count{0};
    u32 m_vertex_count{0};
    vk::IndexType m_index_type = {vtx_raw_data::vk_IndexType<IndexType>()};


    void clear() {
        m_indices.clear();
        m_vertices.clear();
    }
};
using GpuMesh3D = GpuMesh<Vertex3D,u32>;
using GpuMesh2D = GpuMesh<Vertex2D,u32>;
