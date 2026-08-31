#include "vk_buffers.hpp"
#include "vk_types.hpp"
[[nodiscard]] inline auto select_memory_type(
    vk::raii::PhysicalDevice const& m_vkPhysicalDevice,
    u32 type_flags_required,
    vk::MemoryPropertyFlags prop_flags_required
){
    auto device_memory_properties = m_vkPhysicalDevice.getMemoryProperties();
    u32 selected_mem_type_idx{numeric_max<u32>};
    for (u32 idx = 0 ; idx<device_memory_properties.memoryTypeCount; idx++){
        auto const& memoryType  = device_memory_properties.memoryTypes[idx];
        auto const& propertyFlags = memoryType.propertyFlags;

        bool matches_filter = type_flags_required & (1 << idx);
        bool matches_properties = (propertyFlags & prop_flags_required) == prop_flags_required;
        if (!matches_filter){
//               LOG_DBG("Memtype: [{}] does not match mem_type_filter",idx);
        }
        if (!matches_properties){
//                LOG_DBG("Memtype: [{}]{} does not match property flags ({})",idx, vk::to_string(memoryType.propertyFlags),vk::to_string(prop_flags_required));
        }
        if (matches_filter && matches_properties){
            selected_mem_type_idx = idx;
            break;
        }
    }
    if (selected_mem_type_idx == numeric_max<u32>) {
        LOG_FATAL("Unable to find suitable memory type for buffer creation.");
    }else{
//        auto const& memoryType  = device_memory_properties.memoryTypes[selected_mem_type_idx];
//        LOG_DBG(
//            "Selected Memtype: [{}]{} for ({})",
//            selected_mem_type_idx, 
//            vk::to_string(memoryType.propertyFlags),
//            vk::to_string(prop_flags_required)
//        );
    }
    return selected_mem_type_idx;
}

[[nodiscard]] inline auto make_buffer(
    vk::raii::Device const& m_vkDevice,
    vk::raii::PhysicalDevice const& m_vkPhysicalDevice,
    size_t size_bytes,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags memFlags
){
    auto buf = vk::raii::Buffer{
        m_vkDevice,
        {
            .size = size_bytes,
            .usage = usage,
            .sharingMode = vk::SharingMode::eExclusive,
        },
    };
    auto mem_requirements = buf.getMemoryRequirements();
    auto memType = select_memory_type(
        m_vkPhysicalDevice,
        mem_requirements.memoryTypeBits,
        memFlags
    );
    auto memory = vk::raii::DeviceMemory{
        m_vkDevice,
        {
            .allocationSize = mem_requirements.size,
            .memoryTypeIndex = memType,
        },
    };
    
    buf.bindMemory(*memory, 0);

    return std::pair{std::move(buf),std::move(memory)};
}

[[nodiscard]] inline auto make_vertex_buffer(
    vk::raii::Device const& m_vkDevice,
    vk::raii::PhysicalDevice const& m_vkPhysicalDevice,
    size_t size_bytes,
    vk::SharingMode sharing_mode 
){
    return make_buffer(
        m_vkDevice,
        m_vkPhysicalDevice,
        size_bytes,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
}
[[nodiscard]]
inline auto make_uniform_buffer(
    vk::raii::Device const& m_vkDevice,
    vk::raii::PhysicalDevice const& m_vkPhysicalDevice,
    size_t size_bytes,
    vk::SharingMode sharing_mode 
){
    return make_buffer(
        m_vkDevice,
        m_vkPhysicalDevice,
        size_bytes,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal
    );

}
[[nodiscard]]
inline auto make_index_buffer(
    vk::raii::Device const& m_vkDevice,
    vk::raii::PhysicalDevice const& m_vkPhysicalDevice,
    size_t size_bytes, 
    vk::SharingMode sharing_mode 
){
    return make_buffer(
        m_vkDevice,
        m_vkPhysicalDevice,
        size_bytes,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
}
struct GPUBuffer{
    GPUBuffer(
        vk::raii::Device const& m_vkDevice,
        vk::raii::PhysicalDevice const& m_vkPhysicalDevice,
        size_t _size_bytes,
        vk::BufferUsageFlags _usage,
        vk::MemoryPropertyFlags _memFlags
    )
        : size_bytes(_size_bytes)
        , usage(_usage)
        , memFlags(_memFlags)
    {
        std::tie(
            buf,
            memory
        ) = make_buffer(m_vkDevice,m_vkPhysicalDevice,size_bytes,usage,memFlags);
    }
    static inline GPUBuffer make_staging(
        vk::raii::Device const& m_vkDevice,
        vk::raii::PhysicalDevice const& m_vkPhysicalDevice,
        size_t size_bytes
    ){
        return GPUBuffer(
            m_vkDevice,
            m_vkPhysicalDevice,
            size_bytes,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );
    }

    const vk::DeviceSize size_bytes{};
    const vk::BufferUsageFlags usage;
    const vk::MemoryPropertyFlags memFlags{};
    vk::raii::Buffer buf{nullptr};
    vk::raii::DeviceMemory memory{nullptr};

    template<typename T>
    void upload_data(std::span<T> src){
        ASSERT(src.size()>0);
        auto* mapped = memory.mapMemory(0, src.size_bytes(), {});
        std::memcpy(mapped, src.data(), src.size_bytes());
        memory.unmapMemory();
    }
    
};

[[nodiscard]]
inline auto make_staging_buffer(
    vk::raii::Device const& m_vkDevice,
    vk::raii::PhysicalDevice const& m_vkPhysicalDevice,
    size_t size_bytes
){
    return make_buffer(
        m_vkDevice,
        m_vkPhysicalDevice,
        size_bytes,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );
}
