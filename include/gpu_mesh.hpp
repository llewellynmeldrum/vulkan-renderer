#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus

#include "vertex.hpp"
#include "vertex_raw_data.hpp"
#include "vk_managed_buffers.hpp"
template<typename tVertexType, typename tIndexType=u32>
struct GpuMesh{
    using IndexType = tIndexType;
    using VertexType = tVertexType;

    DEFL_CTOR(GpuMesh) = default;
    DEFL_DTOR(GpuMesh) = default;

    MOVE_ASSN(GpuMesh) = default;
    MOVE_CTOR(GpuMesh) = default;

    COPY_ASSN(GpuMesh) = DELETE;
    COPY_CTOR(GpuMesh) = DELETE;

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
