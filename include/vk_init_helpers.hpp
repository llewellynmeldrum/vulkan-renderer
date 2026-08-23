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
inline auto get_pd_queue_family(vk::raii::PhysicalDevice physical_device, Pred&& pred) -> std::optional<std::size_t>{
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
constexpr inline auto get_pd_name(vk::raii::PhysicalDevice const& dev){
    return std::string_view{dev.getProperties().deviceName};
}
constexpr inline auto required_pd_feature_list(){
    return vk::StructureChain{
        vk::PhysicalDeviceFeatures2{
            // 1.0 features
            .features = vk::PhysicalDeviceFeatures{
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
    };
}
constexpr inline auto pd_has_required_features(vk::raii::PhysicalDevice const& pd) {

    auto const features = pd.getFeatures2<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan12Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
    >();

    auto const& f11 = features.get<vk::PhysicalDeviceVulkan11Features>();
    auto const& f12 = features.get<vk::PhysicalDeviceVulkan12Features>();
    auto const& f13 = features.get<vk::PhysicalDeviceVulkan13Features>();
    auto const& ext = features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

    bool f11_compliant = 
        f11.shaderDrawParameters;

    bool f12_compliant = 
        f12.descriptorIndexing 
        && f12.bufferDeviceAddress;

    bool f13_compliant = 
        f13.synchronization2 
        && f13.dynamicRendering;

    bool ext_compliant = ext.extendedDynamicState;
    return f11_compliant && f12_compliant && f13_compliant && ext_compliant;
};
