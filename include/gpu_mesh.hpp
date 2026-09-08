#pragma once 

#include "vertex.hpp"
#include "vk_managed_buffers.hpp"
struct GpuMesh{
    using IndexType = u32;
    using VertexType = Vertex;
    GpuMesh() = default;
    GpuMesh(const GpuMesh &) = delete;
    GpuMesh &operator=(const GpuMesh &) = delete ;
    GpuMesh(GpuMesh &&) = default;
    GpuMesh &operator=(GpuMesh &&) = default;

    VmaAllocator m_allocator{nullptr};
    AllocatedBuffer indices{nullptr};
    AllocatedBuffer vertices{nullptr};
    u32 m_index_count{0};
    u32 m_vertex_count{0};
    vk::IndexType index_type{vk::IndexType::eUint32};


    auto upload_vertices(
        vk::raii::CommandBuffer const& cmdCopyBuf,
        std::span<const Vertex> vertices
    ) -> void;

    void clear() {
        indices.clear();
        vertices.clear();
    }
};
