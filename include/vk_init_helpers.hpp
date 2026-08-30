#pragma once 
#include <algorithm>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include "range/v3/view/enumerate.hpp"
inline bool supports_extension(std::span<vk::ExtensionProperties const> haystack, std::string_view needle) {
    return std::ranges::any_of(
        haystack, 
        [needle](auto const& ext) {
            return std::string_view{ext.extensionName} == needle;
        }
    );
}
template<typename Pred>
inline auto find_matching_queue_family(vk::raii::PhysicalDevice physical_device, Pred&& pred) -> std::optional<std::size_t>{
    auto families = physical_device.getQueueFamilyProperties();
    for (auto const& [idx, family] : ranges::views::enumerate(families)){
        if (!(family.queueFlags & vk::QueueFlagBits::eGraphics)){
            continue;
        }
        if (std::invoke(std::forward<Pred>(pred),idx)){
            return idx;
        }
    }
    return std::nullopt;
}
constexpr inline auto get_physical_dev_name(vk::raii::PhysicalDevice const& dev){
    return std::string_view{dev.getProperties().deviceName};
}
constexpr inline auto enabled_physical_device_features(){
    return vk::StructureChain{
        vk::PhysicalDeviceFeatures2{
            // 1.0 features
            .features = vk::PhysicalDeviceFeatures{
                .fillModeNonSolid = vk::True,
               // .robustBufferAccess = vk::True,
               // .fullDrawIndexUint32 = vk::True,
               // .imageCubeArray = vk::True,
               // .independentBlend = vk::True,
               // .geometryShader = vk::True,
               // .tessellationShader = vk::True,
               // .sampleRateShading = vk::True,
               // .dualSrcBlend = vk::True,
               // .logicOp = vk::True,
               // .multiDrawIndirect = vk::True,
               // .drawIndirectFirstInstance = vk::True,
               // .depthClamp = vk::True,
               // .depthBiasClamp = vk::True,
               // .fillModeNonSolid = vk::True,
               // .depthBounds = vk::True,
               // **UNSUPPORTED ON MVK** .wideLines = vk::True,
               // .largePoints = vk::True,
               // .alphaToOne = vk::True,
               // .multiViewport = vk::True,
               // .samplerAnisotropy = vk::True,
               // .textureCompressionETC2 = vk::True,
               // .textureCompressionASTC_LDR = vk::True,
               // .textureCompressionBC = vk::True,
               // .occlusionQueryPrecise = vk::True,
               // .pipelineStatisticsQuery = vk::True,
               // .vertexPipelineStoresAndAtomics = vk::True,
               // .fragmentStoresAndAtomics = vk::True,
               // .shaderTessellationAndGeometryPointSize = vk::True,
               // .shaderImageGatherExtended = vk::True,
               // .shaderStorageImageExtendedFormats = vk::True,
               // .shaderStorageImageMultisample = vk::True,
               // .shaderStorageImageReadWithoutFormat = vk::True,
               // .shaderStorageImageWriteWithoutFormat = vk::True,
               // .shaderUniformBufferArrayDynamicIndexing = vk::True,
               // .shaderSampledImageArrayDynamicIndexing = vk::True,
               // .shaderStorageBufferArrayDynamicIndexing = vk::True,
               // .shaderStorageImageArrayDynamicIndexing = vk::True,
               // .shaderClipDistance = vk::True,
               // .shaderCullDistance = vk::True,
               // .shaderFloat64 = vk::True,
               // .shaderInt64 = vk::True,
               // .shaderInt16 = vk::True,
               // .shaderResourceResidency = vk::True,
               // .shaderResourceMinLod = vk::True,
               // .sparseBinding = vk::True,
               // .sparseResidencyBuffer = vk::True,
               // .sparseResidencyImage2D = vk::True,
               // .sparseResidencyImage3D = vk::True,
               // .sparseResidency2Samples = vk::True,
               // .sparseResidency4Samples = vk::True,
               // .sparseResidency8Samples = vk::True,
               // .sparseResidency16Samples = vk::True,
               // .sparseResidencyAliased = vk::True,
               // .variableMultisampleRate = vk::True,
               // .inheritedQueries = vk::True,
            },
        },
        vk::PhysicalDeviceVulkan11Features{
            .shaderDrawParameters= vk::True,
        },
        vk::PhysicalDeviceVulkan12Features{
            .descriptorIndexing = vk::True,
            .bufferDeviceAddress = vk::True,
        },
        vk::PhysicalDeviceVulkan13Features{
            .synchronization2 = vk::True,
            .dynamicRendering = vk::True,
        },
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT{
            .extendedDynamicState = vk::True,
        },
        vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT{
            .extendedDynamicState3PolygonMode = vk::True,
        },
    };
}
