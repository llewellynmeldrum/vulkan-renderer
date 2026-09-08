#pragma once 

#include "SDL3/SDL_vulkan.h"
#include "SDL3/SDL_video.h"
#include "Texture2D.hpp"
#include "vk_managed_buffers.hpp"
#include "renderer_buffer_helpers.hpp"
#include "vk_frame_data.hpp"
#include "vk_types.hpp"
#include "renderer_init_helpers.hpp"
#include "vk_debug.hpp"
#include "vk_swapchain.hpp"

namespace detail{
// Private header file included only by the main initialization cpp file.

struct InstanceContext{
    vk::raii::Instance m_vkInstance{nullptr};
    vk::raii::DebugUtilsMessengerEXT m_vkDebugMessenger {nullptr};
};
[[nodiscard]] inline auto
make_vk_instance(
    bool m_useValidationLayers,
    vk::raii::Context const& m_vkContext,
    u32 API_VER,
    char const* app_name="n/a",
    char const* engine_name="n/a"
) -> InstanceContext
{
    // sdl requires some extensions for its windowing stuff
    auto ctx = InstanceContext{};
    u32 ext_count{};
    auto const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&ext_count);
    if (!sdl_extensions){
        LOG_FATAL("Failed to query extensions from SDL. SDL_Error:{}",SDL_GetError());
    }
    auto instanceExtensions = std::vector<char const*>(sdl_extensions, sdl_extensions+ext_count);
    auto appLayers = std::vector<char const*>{};
    auto instanceFlags = vk::InstanceCreateFlags{};


    struct InstanceExtension{
        const char* name;
        vk::InstanceCreateFlagBits flag;
    };
    static constexpr InstanceExtension portabilityKHR{
        .name = vk::KHRPortabilityEnumerationExtensionName,
        .flag = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
    };

    auto const inst_supported_extensions = m_vkContext.enumerateInstanceExtensionProperties();
    // this is required to enable moltenVK support, as it is a non conformant driver
    if (supports_extension(inst_supported_extensions, portabilityKHR.name)){
        instanceExtensions.push_back(portabilityKHR.name);
        instanceFlags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
    }

    constexpr auto VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";
    if (m_useValidationLayers){
        appLayers.push_back(VALIDATION_LAYER);
        instanceExtensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

 
    auto appInfo = vk::ApplicationInfo{
        .pApplicationName = app_name,
        .applicationVersion = vk::makeApiVersion(0, 0, 1, 0),
        .pEngineName = engine_name,
        .engineVersion = vk::makeApiVersion(0, 0, 1, 0),
        .apiVersion = API_VER,
    };

    using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;
    auto const instanceDebugInfo = vk::DebugUtilsMessengerCreateInfoEXT{
        .messageSeverity = Severity::eWarning | Severity::eError,
        .messageType = Type::eGeneral | Type::eValidation | Type::ePerformance,
        .pfnUserCallback = detail::debug::vk_debug_callback,
    };

    ctx.m_vkInstance = vk::raii::Instance{
        m_vkContext,
        vk::InstanceCreateInfo{}
            .setPNext(m_useValidationLayers ? &instanceDebugInfo : nullptr)
            .setFlags(instanceFlags)
            .setPApplicationInfo(&appInfo)
            .setPEnabledLayerNames(appLayers)
            .setPEnabledExtensionNames(instanceExtensions)
    };
    if (m_useValidationLayers){
        ctx.m_vkDebugMessenger = vk::raii::DebugUtilsMessengerEXT{ctx.m_vkInstance, instanceDebugInfo};
    }
    return ctx;
}

[[nodiscard]] inline auto
make_vk_surface(
    vk::raii::Instance const& instance,
    SDL_Window* window
) -> vk::raii::SurfaceKHR
{
    ASSERT(window);

    // Have SDL create the surface given our instance we just setup 
    auto raw_surface = VkSurfaceKHR{};
    if (!SDL_Vulkan_CreateSurface(window, detail::debug::get_c_handle(instance), nullptr,
                                  &raw_surface)) {
        LOG_FATAL("Failure in {}(): {}", "SDL_Vulkan_CreateSurface", SDL_GetError());
    }
    return vk::raii::SurfaceKHR{instance, raw_surface};
}

struct DeviceQueueContext{
    vk::raii::PhysicalDevice m_vkPhysicalDevice{nullptr};
    vk::raii::Device         m_vkDevice{nullptr};
    vk::raii::Queue          m_vkQueue{nullptr};
    u32                      m_vkQueueFamily{};
};
[[nodiscard]] inline auto
make_vk_device_and_queue(
    vk::raii::Instance const& m_vkInstance,
    vk::raii::SurfaceKHR const& m_vkSurface,
    u32 API_VER
) -> DeviceQueueContext
{
    // NOTE: Instance extensions modify global behaviour BEFORE a device is selected,
    // whereas DEVICE extensions modify the behaviour of a specific vk::Device.

    DeviceQueueContext ctx{};
    // Configure Device extensions
    auto device_extensions = std::vector<char const*>{};
    // These extensions are required. We cannot function properly without them.
    static constexpr auto required_device_extensions = std::array{
        vk::KHRSwapchainExtensionName,
        vk::EXTExtendedDynamicState3ExtensionName,
        vk::EXTMemoryBudgetExtensionName,
    };

    // iterate over all physical devices listed by driver
    for (auto && physical_device : m_vkInstance.enumeratePhysicalDevices()){
        // skip if device doesnt support expected api version
        auto device_api_ver = physical_device.getProperties().apiVersion;
        if (device_api_ver < API_VER) continue;

        auto const supported_extensions = physical_device.enumerateDeviceExtensionProperties();
        bool device_supports_required_extensions {true};
        for (auto const& required_ext: required_device_extensions){
            if (supports_extension(supported_extensions,required_ext)){
                device_extensions.push_back(required_ext); 
            }else{
                device_supports_required_extensions = false;
                break;
            }
        }
        if (!device_supports_required_extensions){
            continue;
        }
        static constexpr auto EXT_MOLTENVK_FIX = "VK_KHR_portability_subset";
        // 'VK_KHR_portability_subset must be enabled if physical device supports it.'
        if (supports_extension(supported_extensions,EXT_MOLTENVK_FIX)){
            device_extensions.push_back(EXT_MOLTENVK_FIX);
        }

        auto family = find_matching_queue_family(
            physical_device, 
            [&](u32 idx){
                return physical_device.getSurfaceSupportKHR(idx,m_vkSurface) == vk::True;
            }
        );
        if (!family) continue; // no matching queue family on the device

        auto queue_prio = 1.0f;
        auto queue_create_info = vk::DeviceQueueCreateInfo{}
            .setQueueFamilyIndex(*family)
            .setQueueCount(1)
            .setQueuePriorities(queue_prio)
        ;

        
        auto enabled_features = enabled_physical_device_features();
        try {
            ctx.m_vkDevice = vk::raii::Device{
                physical_device,
                vk::DeviceCreateInfo{}
                    .setPNext(&enabled_features.get<vk::PhysicalDeviceFeatures2>())
                    .setQueueCreateInfos(queue_create_info)
                    .setPEnabledExtensionNames(device_extensions)
            };
        } catch (vk::FeatureNotPresentError e){
            LOG_FATAL("PD {} is missing a feature: {}", get_physical_dev_name(physical_device),e.what());
            continue;
        }

        ctx.m_vkPhysicalDevice = std::move(physical_device);
        ctx.m_vkQueueFamily = *family;
        break;
    }

    if (!*ctx.m_vkPhysicalDevice){
        LOG_FATAL("Unable to select a physical device! Driver listed {}, none matched",//
                  m_vkInstance.enumeratePhysicalDevices().size());
    }else{
        LOG_INFO("Device created, PD selected: {}", get_physical_dev_name(ctx.m_vkPhysicalDevice));
    }

    ctx.m_vkQueue = ctx.m_vkDevice.getQueue(ctx.m_vkQueueFamily, 0);
    return ctx;
}
[[nodiscard]] inline auto
make_vma_allocator(
    vk::raii::PhysicalDevice const& m_vkPhysicalDevice,
    vk::raii::Device const& m_vkDevice,
    vk::raii::Instance const& m_vkInstance
) -> VmaAllocator
{
    auto m_allocator = VmaAllocator{};
    auto vma_vulkan_functions = VmaVulkanFunctions{
        .vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr )SDL_Vulkan_GetVkGetInstanceProcAddr(),
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
    };
    auto create_info = VmaAllocatorCreateInfo{
        .flags =  VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
        .physicalDevice = *m_vkPhysicalDevice,
        .device = *m_vkDevice,
        .pVulkanFunctions = &vma_vulkan_functions,
        .instance = *m_vkInstance,
        .vulkanApiVersion = VK_API_VERSION_1_4,
    };

    auto res = vmaCreateAllocator(&create_info, &m_allocator);
    if (res != VkResult::VK_SUCCESS){
        LOG_FATAL("Vulkan Memory Allocator failed to initialize");
    }
    return m_allocator;
}

[[nodiscard]] inline auto
make_swapchain(
    SwapchainSettings in
) -> Swapchain
{
    auto fmts = in.physical_device.getSurfaceFormatsKHR(in.surface);
    ASSERT(fmts.size() > 0);

    auto preferred = vk::SurfaceFormatKHR{in.format,in.colorSpace};
    auto surface_supports_fmt = std::ranges::contains(fmts, preferred) ;
    auto image_fmt = 
        surface_supports_fmt
        ? *(std::ranges::find(fmts,preferred))
        : fmts.front();
    if (!surface_supports_fmt){
        LOG_WARN("Surface does not support desired format ({}/{}), falling back to {}/{}.",
                 vk::to_string(in.format),vk::to_string(in.format),
                 vk::to_string(image_fmt.format),vk::to_string(image_fmt.format)
                 );
    }

    auto modes = in.physical_device.getSurfacePresentModesKHR(in.surface);
    bool surface_supports_present_mode = std::ranges::contains(modes,in.present_mode);
    auto present_mode = surface_supports_present_mode
            ? in.present_mode
            : vk::PresentModeKHR::eFifo; // only one guaranteed by the spec
    if (!surface_supports_present_mode){
        LOG_WARN("Surface does not support desired present mode({}), falling back to {}." ,
                 vk::to_string(in.present_mode),
                 vk::to_string(vk::PresentModeKHR::eFifo));
    }

    auto caps = in.physical_device.getSurfaceCapabilitiesKHR(in.surface);
    auto extent = in.extent_px;
    static constexpr auto UNDEFINED_EXTENT {numeric_max<u32>};
    bool surface_has_fixed_extents = caps.currentExtent.width != UNDEFINED_EXTENT;
    if (!surface_has_fixed_extents){
        extent=in.extent_px;
        auto const max = caps.maxImageExtent;
        auto const min = caps.minImageExtent;
        extent.width = std::clamp(extent.width, min.width, max.width);
        extent.height = std::clamp(extent.height, min.height, max.height);
    }

    auto image_count = caps.minImageCount+1;
    if (caps.maxImageCount > 0) { // 0 means "no upper bound" apparently
        image_count = std::min(image_count, caps.maxImageCount);
    }
    LOG_DBG("caps.maxImageCount:{}",caps.maxImageCount);
    LOG_DBG("caps.minImageCount:{}",caps.minImageCount);
    LOG_DBG("image_count:{}",image_count);
    // next is checking usage flags
    if ((caps.supportedUsageFlags & in.image_usage_flags) != in.image_usage_flags){
        LOG_FATAL("Surface does not support desired image usage {} (supports {})",
                  vk::to_string(in.image_usage_flags),
                  vk::to_string(caps.supportedUsageFlags)
                  );
    }
    auto swapchain = Swapchain{
        .descriptor = vk::raii::SwapchainKHR{in.device,
            vk::SwapchainCreateInfoKHR{
                .surface = *in.surface,
                .minImageCount = image_count,
                .imageFormat = image_fmt.format,
                .imageColorSpace = image_fmt.colorSpace,
                .imageExtent = extent,
                .imageArrayLayers = 1,
                .imageUsage = in.image_usage_flags,
                .imageSharingMode = vk::SharingMode::eExclusive,
                .preTransform = caps.currentTransform,
                .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
                .presentMode = present_mode,
                .clipped = vk::True,
            },
        },
        .imageFormat = image_fmt.format,
        .extent = extent,
    };

    swapchain.images = swapchain.descriptor.getImages();
    ASSERT(swapchain.images.size() > 0);

    swapchain.imageViews.reserve(swapchain.images.size());
    int swap_img_idx = 0;
    for (auto const& image: swapchain.images){
        set_vk_dbg_name(in.device,image, std::format("Swapchain image (frame:{})",swap_img_idx++));
        swapchain.imageViews.emplace_back(
            in.device,
            vk::ImageViewCreateInfo{
                .image = image,
                .viewType = vk::ImageViewType::e2D,
                .format = swapchain.imageFormat,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .levelCount = 1,
                    .layerCount = 1,
                },
            }
        );
        swapchain.renderFinishedSemaphores.emplace_back(
            in.device,
            vk::SemaphoreCreateInfo{}
        );
    }
    int idx = 0;
    for (const auto& rend_sem: swapchain.renderFinishedSemaphores){
        set_vk_dbg_name(in.device, rend_sem, std::format("({}) render sem", idx++));
    }
    return swapchain;
}

[[nodiscard]] inline auto
make_inflight_frames(
    vk::raii::Device const& m_vkDevice,
    vk::raii::PhysicalDevice const& m_vkPhysicalDevice,
    u32 frameCount,
    u32 m_vkQueueFamily
)-> std::vector<FrameData>
{
    std::vector<FrameData> m_inflightFrames{}; 
    m_inflightFrames.resize(frameCount);
    int frame_idx = 0;
    for (auto& frame : m_inflightFrames){
        frame.commandPool = vk::raii::CommandPool{
            m_vkDevice,
            vk::CommandPoolCreateInfo{
                // without this flag, we would not be able to individually reset command buffers,
                // but rather only the whole pool
                .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = m_vkQueueFamily,
            },
        };
        
        auto buffers = m_vkDevice.allocateCommandBuffers(
            vk::CommandBufferAllocateInfo{
                .commandPool = *frame.commandPool,
                // Primary can be submitted to a queue for execution,
                // whilst secondary can be called from primary cmd buffers
                .level = vk::CommandBufferLevel::ePrimary, 
                .commandBufferCount = 1,
            }
        );
        ASSERT(buffers.size() == 1);
        frame.commandBuffer = std::move(buffers.at(0));
        set_vk_dbg_name(m_vkDevice,frame.commandBuffer, std::format("Frame[{}] command buffer",frame_idx));
        frame_idx++;

        // Create the uniform buffers for each frame, based on the size of the UniformBufferObject struct
        static constexpr auto ubo_size_bytes = static_cast<u32>(sizeof(UniformBufferObject));
        std::tie(
            frame.uniformBuffer,
            frame.uniformBufferMemory 
        ) = detail::helpers::make_uniform_buffer(
                m_vkDevice,
                m_vkPhysicalDevice,
                ubo_size_bytes,
                vk::SharingMode::eExclusive
            );
        frame.uniformBufferMappedMemory = frame.uniformBufferMemory.mapMemory(0,ubo_size_bytes);
    }

    return m_inflightFrames;
}

[[nodiscard]] inline auto
make_descriptor_set_layout(
    vk::raii::Device const& m_vkDevice
) -> vk::raii::DescriptorSetLayout 
{
    // Descriptor set bindings all combine into a single desciptor set layout.
    auto layoutBindings = std::array{
        vk::DescriptorSetLayoutBinding{
            vk::DescriptorSetLayoutBinding{}
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(1)
                .setStageFlags(vk::ShaderStageFlagBits::eVertex)
        },
        vk::DescriptorSetLayoutBinding{
            vk::DescriptorSetLayoutBinding{}
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setDescriptorCount(1)
                .setStageFlags(vk::ShaderStageFlagBits::eFragment)
        },
    };
    for (i32 i = 0; i<layoutBindings.size(); i++){
        layoutBindings[i].setBinding(i);
    }

    return vk::raii::DescriptorSetLayout{
        m_vkDevice, 
        vk::DescriptorSetLayoutCreateInfo{}
            .setBindings(layoutBindings)
    };
}

[[nodiscard]] inline auto
make_descriptor_pool(
    vk::raii::Device const& m_vkDevice,
    u32 syncFrameCount
)-> vk::raii::DescriptorPool 
{
    auto poolSizes = std::array{
        vk::DescriptorPoolSize{}
            .setType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(syncFrameCount),
        vk::DescriptorPoolSize{}
            .setType(vk::DescriptorType::eCombinedImageSampler)
            .setDescriptorCount(syncFrameCount),
    };
    return vk::raii::DescriptorPool{
        m_vkDevice,
        vk::DescriptorPoolCreateInfo{}
            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets(syncFrameCount)
            .setPoolSizes(poolSizes)
    };
}

[[nodiscard]] inline auto 
make_descriptor_sets(
    vk::raii::Device const& m_vkDevice,
    vk::raii::DescriptorPool const& m_vkDescriptorPool,
    vk::raii::DescriptorSetLayout const& m_vkDescriptorSetLayout,
    std::span<const FrameData> m_inflightFrames,
    Texture2D const& texture,
    u32 syncFrameCount
) -> std::vector<vk::raii::DescriptorSet>
{
    auto layouts = std::vector<vk::DescriptorSetLayout>(syncFrameCount, m_vkDescriptorSetLayout); 
    ASSERT(layouts.size() == syncFrameCount);

    auto res = m_vkDevice.allocateDescriptorSets(
        vk::DescriptorSetAllocateInfo{}
            .setDescriptorPool(m_vkDescriptorPool)
            .setSetLayouts(layouts)
    );
    for (auto frame_idx = 0uz; frame_idx<syncFrameCount; frame_idx++){
        auto bufferInfo = 
            vk::DescriptorBufferInfo{}
                .setBuffer(*m_inflightFrames[frame_idx].uniformBuffer)
                .setOffset(0)
                .setRange(sizeof(UniformBufferObject))
         ;
        auto imageInfo = 
            vk::DescriptorImageInfo{}
            .setSampler(texture.sampler)
            .setImageView(texture.image_view)
            .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
         ;
        auto descriptorWriteSets = std::array{
            vk::WriteDescriptorSet{}
                .setDstSet(*res[frame_idx])
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setBufferInfo(bufferInfo)
            ,
            vk::WriteDescriptorSet{}
                .setDstSet(*res[frame_idx])
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setImageInfo(imageInfo)
        };
        for (i32 i = 0; i<descriptorWriteSets.size(); i++){
            descriptorWriteSets[i].setDstBinding(i);
        }
        m_vkDevice.updateDescriptorSets(
            descriptorWriteSets,
            {}
        );
    }
    return res;
}











}// namespace detail
