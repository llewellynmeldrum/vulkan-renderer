#pragma once 

#include "vk_managed_buffers.hpp"
struct GpuMesh{
    GpuMesh() = default;
    GpuMesh(const GpuMesh &) = delete;
    GpuMesh &operator=(const GpuMesh &) = delete ;
    GpuMesh(GpuMesh &&) = default;
    GpuMesh &operator=(GpuMesh &&) = default;

    VmaAllocator allocator{nullptr};
    AllocatedBuffer indices{nullptr};
    AllocatedBuffer vertices{nullptr};
    u32 m_index_count{0};
    u32 num_vertices{0};
    vk::IndexType index_type{vk::IndexType::eUint32};
    void clear() {
        indices.clear();
        vertices.clear();
    }
};
