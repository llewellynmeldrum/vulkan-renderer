#include "gpu_mesh.hpp"
#include "vulkan/vulkan.hpp"

auto GpuMesh::upload_vertices(
    vk::raii::CommandBuffer const& cmdCopyBuf,
    std::span<const Vertex> in_vertices
) -> void{
    auto vtx_buf_size_bytes = in_vertices.size() * sizeof(VertexType);

    // 1. make mapped, cpu accessible staging buffer
//    LOG_DBG_EXPR(vtx_buf_size_bytes);
//    LOG_DBG_EXPR(idx_buf_size_bytes);
    auto stagingBuffer = AllocatedBuffer(m_allocator,
        vk::BufferCreateInfo{}
            .setSize(vtx_buf_size_bytes)
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
    auto vertex_cpu_to_staging_res = VkResult{};
    vertex_cpu_to_staging_res = vmaCopyMemoryToAllocation(
        m_allocator,
        in_vertices.data(),
        stagingBuffer.allocation,
        0,
        vk::DeviceSize{vtx_buf_size_bytes}
    );
    if (vertex_cpu_to_staging_res != VkResult::VK_SUCCESS){
        LOG_FATAL("Failed to copy cpu vertex data into staging buffer.");
    }

    this->vertices = AllocatedBuffer(m_allocator,
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

    cmdCopyBuf.copyBuffer(
        stagingBuffer.buffer,
        vertices.buffer,
        vk::BufferCopy{.srcOffset=0, .dstOffset = 0, .size=vtx_buf_size_bytes}
    );
    stagingBuffer.clear();
}
