#include "glm_types.hpp"
#include "vk_engine.hpp"
#include "vk_types.hpp"
GpuMesh VkEngine::upload_gpu_mesh(CpuMesh const& cpu_mesh){
    auto mesh = GpuMesh{};
    mesh.allocator = m_allocator;
    mesh.num_vertices = cpu_mesh.vertices.size();
    mesh.m_index_count = cpu_mesh.indices.size();
//    LOG_DBG("Uploading gpu mesh ({} verts, {} indices, {} quads.)",mesh.num_vertices, mesh.m_index_count, cpu_mesh.m_quad_count);


    auto vtx_buf_size_bytes = cpu_mesh.vertices.size() * sizeof(CpuMesh::VertexType);
    auto idx_buf_size_bytes = cpu_mesh.indices.size() * sizeof(CpuMesh::IndexType);

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
    auto res = VkResult{};
    res = vmaCopyMemoryToAllocation(
        m_allocator,
        cpu_mesh.vertices.data(),
        stagingBuffer.allocation,
        0,
        vk::DeviceSize{vtx_buf_size_bytes}
    );
    if (res != VkResult::VK_SUCCESS){
        LOG_FATAL("Failed to copy cpu vertex data into staging buffer.");
    }
    res = vmaCopyMemoryToAllocation(
        m_allocator,
        cpu_mesh.indices.data(),
        stagingBuffer.allocation,
        vtx_buf_size_bytes, // offset, begins after vtx data
        idx_buf_size_bytes
    );
    if (res != VkResult::VK_SUCCESS){
        LOG_FATAL("Failed to copy cpu index data into staging buffer.");
    }

    // 2. create the vertex and index buffer
    mesh.vertices = AllocatedBuffer(m_allocator,
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
    mesh.indices = AllocatedBuffer(m_allocator,
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
    copy_buffer(stagingBuffer.buffer, mesh.vertices.buffer, vtx_buf_size_bytes, 0);
    copy_buffer(stagingBuffer.buffer, mesh.indices.buffer, idx_buf_size_bytes, vtx_buf_size_bytes);


    return mesh;
}
