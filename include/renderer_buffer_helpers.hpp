#pragma once 
#include "file_io.hpp"
#include "vk_managed_buffers.hpp"
#include "vk_types.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>
namespace detail::helpers{
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
    auto memType = detail::helpers::select_memory_type(
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

[[nodiscard]] inline auto 
make_uniform_buffer(
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
struct ShaderModuleWrapper{
    vk::raii::ShaderModule module;
    std::string_view spirv_source_name;
    std::string_view module_name;
};
[[nodiscard]] inline auto make_shader_module(
    vk::raii::Device const& m_vkDevice,
    std::string_view spirv_filename,
    std::string_view  module_name
)-> ShaderModuleWrapper{
    auto spirv_src = read_file_contents(spirv_filename);
    return ShaderModuleWrapper{
            vk::raii::ShaderModule{
                m_vkDevice,
                vk::ShaderModuleCreateInfo{
                    .codeSize = spirv_src.size(),
                    .pCode = reinterpret_cast<u32 const*>(spirv_src.data()),
                }
            },
            spirv_filename,
            module_name,
    };
}
[[nodiscard]] inline auto no_blend() -> vk::PipelineColorBlendAttachmentState
{
    return vk::PipelineColorBlendAttachmentState{
        .blendEnable = vk::False,
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha, 
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,

        .colorBlendOp        = vk::BlendOp::eAdd,

        .srcAlphaBlendFactor = vk::BlendFactor::eOne, .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp        = vk::BlendOp::eAdd,

        .colorWriteMask = vk::ColorComponentFlagBits::eR 
            | vk::ColorComponentFlagBits::eG
            | vk::ColorComponentFlagBits::eB
            | vk::ColorComponentFlagBits::eA
    };
}

[[nodiscard]] inline auto alpha_blend() -> vk::PipelineColorBlendAttachmentState
{
    return vk::PipelineColorBlendAttachmentState{
        .blendEnable = vk::True,
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha, 
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,

        .colorBlendOp        = vk::BlendOp::eAdd,

        .srcAlphaBlendFactor = vk::BlendFactor::eOne, 
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,

        .alphaBlendOp        = vk::BlendOp::eAdd,

        .colorWriteMask = vk::ColorComponentFlagBits::eR 
            | vk::ColorComponentFlagBits::eG
            | vk::ColorComponentFlagBits::eB
            | vk::ColorComponentFlagBits::eA
    };
}

} // namespace detail::helpers
