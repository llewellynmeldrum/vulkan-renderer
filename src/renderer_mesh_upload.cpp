#include "glm_types.hpp"
#include "renderer.hpp"
#include "vk_types.hpp"

auto Renderer::upload_mesh2d() -> void {
    m_rend2d.m_gpu_mesh = make_gpu_mesh(m_rend2d.m_cpu_mesh);
}
auto Renderer::upload_mesh3d(
    MeshID id,
    CpuMesh3D cpu_mesh,
    glm::mat4x4 model_matrix
) -> void {
    LOG_DBG("Uploading mesh id={}, vtx:{},idx:{}",id, cpu_mesh.vertices.size(), cpu_mesh.indices.size());
    m_model_matrices.insert_or_assign(id, model_matrix);
    auto gpu_slot = m_gpu_meshes3d.find(id);
    bool mesh_already_on_gpu = gpu_slot != m_gpu_meshes3d.end();
    if (mesh_already_on_gpu){
        // naiive approach, delete the old one and replace it entirely
        gpu_slot->second.clear();
        m_gpu_meshes3d.insert_or_assign(gpu_slot, id, make_gpu_mesh(cpu_mesh));
    } else{
        auto [it, inserted] = m_gpu_meshes3d.try_emplace(id, make_gpu_mesh(cpu_mesh));
        ASSERT(inserted);
    }
}

template<typename tVertexType, typename tIndexType>
auto Renderer::make_gpu_mesh_impl(CpuMesh<tVertexType,tIndexType> const& cpu_mesh) -> GpuMesh<tVertexType,tIndexType>{
    using GpuMeshType = GpuMesh<tVertexType,tIndexType>;
    using VertexType = tVertexType;
    using IndexType = tIndexType;
    auto mesh = GpuMeshType{};
    mesh.m_allocator = m_allocator;
    mesh.m_vertex_count = cpu_mesh.vertices.size();
    mesh.m_index_count = cpu_mesh.indices.size();
    auto vtx_buf_size_bytes = cpu_mesh.vertices.size() * sizeof(VertexType);
    auto idx_buf_size_bytes = cpu_mesh.indices.size() * sizeof(IndexType);

    // 1. make mapped, cpu accessible staging buffer
//    LOG_DBG_EXPR(vtx_buf_size_bytes);
//    LOG_DBG_EXPR(idx_buf_size_bytes);
    auto stagingBuffer = AllocatedBuffer(m_allocator,
        vk::BufferCreateInfo{}
            .setSize(vtx_buf_size_bytes + idx_buf_size_bytes)
            .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
            .setSharingMode(vk::SharingMode::eExclusive)
        ,
        VmaAllocationCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        },
        "Staging buf"
    );
    ASSERT(stagingBuffer.mapped_ptr);
    // 2. Copy the data into the staging buffer

    // NOTE:================================
    // Mapping into staging buffer will be: 
    // =====================================
    // |------------|-----------|
    // | vertices   | indices   |
    // |------------|-----------|
    // ^            ^           ^
    // |            |           vtx_buf_size_bytes+idx_buf_size_bytes
    // |            vtx_buf_size_bytes
    // 0x0 
    // So we can reuse a single staging buffer.
    auto vertex_cpu_to_staging_res = VkResult{};
    vertex_cpu_to_staging_res = vmaCopyMemoryToAllocation(
        m_allocator,
        cpu_mesh.vertices.data(),
        stagingBuffer.allocation,
        0,
        vk::DeviceSize{vtx_buf_size_bytes}
    );
    if (vertex_cpu_to_staging_res != VkResult::VK_SUCCESS){
        LOG_FATAL("Failed to copy cpu vertex data into staging buffer.");
    }
    auto index_cpu_to_staging_res = VkResult{};
    index_cpu_to_staging_res = vmaCopyMemoryToAllocation(
        m_allocator,
        cpu_mesh.indices.data(),
        stagingBuffer.allocation,
        vtx_buf_size_bytes, // offset, begins after vtx data
        idx_buf_size_bytes
    );
    if (index_cpu_to_staging_res != VkResult::VK_SUCCESS){
        LOG_FATAL("Failed to copy cpu index data into staging buffer.");
    }

    // 2. create the vertex and index buffer
    mesh.m_vertices = AllocatedBuffer(m_allocator,
        vk::BufferCreateInfo{}
            .setSize(vtx_buf_size_bytes)
            .setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst)
            .setSharingMode(vk::SharingMode::eExclusive)
        ,
        VmaAllocationCreateInfo{
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        },
        "mesh VB"
    );
    mesh.m_indices = AllocatedBuffer(m_allocator,
        vk::BufferCreateInfo{}
            .setSize(idx_buf_size_bytes)
            .setUsage(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst)
            .setSharingMode(vk::SharingMode::eExclusive)
        ,
        VmaAllocationCreateInfo{
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        },
        "mesh IB"

    );
    // 3. copy data from staging buffer into vertex and index buffer
    copy_buffer(stagingBuffer.buffer, mesh.m_vertices.buffer, vtx_buf_size_bytes, 0);
    copy_buffer(stagingBuffer.buffer, mesh.m_indices.buffer, idx_buf_size_bytes, vtx_buf_size_bytes);
    stagingBuffer.clear();
    return mesh;
}
auto Renderer::make_gpu_mesh(CpuMesh2D const& cpu_mesh) -> GpuMesh2D {
    return make_gpu_mesh_impl(cpu_mesh);
}
auto Renderer::make_gpu_mesh(CpuMesh3D const& cpu_mesh) -> GpuMesh3D {
    return make_gpu_mesh_impl(cpu_mesh);
}
