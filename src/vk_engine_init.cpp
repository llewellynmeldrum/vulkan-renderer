#include <thread>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#include "SDL3/SDL_vulkan.h"
#include "heightmap.hpp"
#include "sdl3_types.hpp"
#include "glm_types.hpp"

#include "stb_image.hpp"
#include "format_specs.hpp"
#include "file_io.hpp"
#include "shared.hpp"
#include "timer.hpp"
#include "vertex.hpp"
#include "vk_engine.hpp"
#include "vertex_raw_data.hpp"


#include "vk_debug.hpp"
#include "vk_types.hpp"
#include "vk_util.hpp"
#include "vk_buffers.hpp"
#include "vk_init_helpers.hpp"
#include "vk_buffers_helpers.hpp"

static char const* APP_NAME = "Test Window";

void VkEngine::init() {
    LOG_INFO("INITIALIZING ENGINE ({})", static_cast<void*>(this));
    ASSERT(m_loadedEngine == nullptr);
    m_loadedEngine = this;
    try{
        init_sdl();
        init_vk_instance();
        init_vk_surface();
        init_vk_device_and_queue();
        init_vma();
        init_swapchain();
        init_inflightFrames();
        init_buffers();  //  TODO: refactor to be like the others
        init_textures(); //  TODO: refactor to be like the others
        init_depth_attachment(); //  TODO: refactor to be like the others
        init_descriptor_set_layout();
        init_descriptor_pool();
        init_descriptor_sets(); //  TODO: refactor to be like the others
        init_pipeline();  //  TODO: refactor to be like the others
        init_sync_structures(); //  TODO: refactor to be like the others
        init_heightmap();
        upload_heightmap();
    }catch(vk::SystemError const& e){
        LOG_FATAL("Failed to initialize vulkan: {}", e.what());
    }
}
[[nodiscard]]
SDL_Window* make_window(vk::Extent2D m_windowLogicalSize){
    SDL_Window* m_window{nullptr};
    static constexpr auto init_flags = 
        SDL_INIT_VIDEO
        | SDL_INIT_EVENTS;

    static constexpr auto window_flags = 
        SDL_WINDOW_VULKAN 
        | SDL_WINDOW_RESIZABLE
        | SDL_WINDOW_HIGH_PIXEL_DENSITY;


    SDL_Init(init_flags);
    m_window = SDL_CreateWindow(
        APP_NAME,
        m_windowLogicalSize.width,
        m_windowLogicalSize.height,
        window_flags
    );

    if (!m_window) {
        LOG_ERROR("Failed to init window : {}", SDL_GetError());
        LOG_EXIT(1);
    }
    if (!SDL_SetWindowRelativeMouseMode(m_window, true)){
        LOG_ERROR("Failed to set relative mouse mode: {}", SDL_GetError());
        LOG_EXIT(1);
    }
    return m_window;
}

void VkEngine::init_sdl() {
    m_window = make_window(m_windowLogicalSize);
}

void VkEngine::upload_heightmap() {
    m_gpu_heightmapMesh = upload_gpu_mesh(m_cpu_heightmapMesh);
}

void VkEngine::init_heightmap() {
    f32 ex = 100.0f;
    f32 ez = 100.0f;
    m_heightmap = Heightmap(
        HeightmapCreateInfo{
            .world_center = glm::vec3{2.0f,2.0f, 4.0f},
            .extentX = ex,
            .extentZ = ez,
            .noise_freq = 0.1f,
            .boundsY = {-1,+1}
        }
    );
    m_cpu_heightmapMesh = mesh_heightmap(
        m_heightmap,
        HeightMapMeshCreateInfo{
            .num_x_samples = static_cast<u32>(64 * ex),
            .num_z_samples = static_cast<u32>(64 * ez),
        }
    );
}
[[nodiscard]]
Swapchain make_swapchain(
    SwapchainSettings in
){
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
void VkEngine::init_swapchain() {
    m_swapchain = make_swapchain(
        SwapchainSettings{
            .physical_device = m_vkPhysicalDevice,
            .device = m_vkDevice,
            .surface = m_vkSurface,
            .extent_px = get_framebuffer_size(),
        }
    );
    
}


void VkEngine::init_buffers() {
    // UNIFORM BUFFERS
    for (auto& frame : m_inflightFrames){
        auto const bufSize = static_cast<u32>(sizeof(UniformBufferObject));
        std::tie(
            frame.uniformBuffer,
            frame.uniformBufferMemory 
        ) = make_uniform_buffer(m_vkDevice, m_vkPhysicalDevice, bufSize,vk::SharingMode::eExclusive);
        frame.uniformBufferMappedMemory = frame.uniformBufferMemory.mapMemory(0,bufSize);
    }

}
struct DeviceQueueContext{
    vk::raii::PhysicalDevice m_vkPhysicalDevice{nullptr};
    vk::raii::Device         m_vkDevice{nullptr};
    vk::raii::Queue          m_vkQueue{nullptr};
    u32                      m_vkQueueFamily{};
};
[[nodiscard]]
DeviceQueueContext make_vk_device_and_queue(
    vk::raii::Instance const& m_vkInstance,
    vk::raii::SurfaceKHR const& m_vkSurface,
    u32 API_VER
){
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

void VkEngine::init_vk_device_and_queue(){
    auto ctx = make_vk_device_and_queue(m_vkInstance,m_vkSurface,API_VER);
    m_vkPhysicalDevice = std::move(ctx.m_vkPhysicalDevice);
	m_vkDevice = std::move(ctx.m_vkDevice);
	m_vkQueue = std::move(ctx.m_vkQueue);
	m_vkQueueFamily = std::move(ctx.m_vkQueueFamily);
}
[[nodiscard]]
auto make_vk_surface(
    vk::raii::Instance const& instance,
    SDL_Window* window
){
    ASSERT(window);
    // Have SDL create the surface given our instance we just setup 
    auto raw_surface = VkSurfaceKHR{};
    if (!SDL_Vulkan_CreateSurface(window, get_c_handle(instance), nullptr,
                                  &raw_surface)) {
        LOG_FATAL("Failure in {}(): {}", "SDL_Vulkan_CreateSurface", SDL_GetError());
    }
    return vk::raii::SurfaceKHR{instance, raw_surface};
}
void VkEngine::init_vk_surface(){
    m_vkSurface = make_vk_surface(m_vkInstance, m_window);
}

struct VkInstanceContext{
    vk::raii::Instance m_vkInstance{nullptr};
    vk::raii::DebugUtilsMessengerEXT m_vkDebugMessenger {nullptr};
    
};
[[nodiscard]]
VkInstanceContext make_vk_instance(
    bool m_useValidationLayers,
    vk::raii::Context const& m_vkContext,
    u32 API_VER
){
    // sdl requires some extensions for its windowing stuff
    auto ctx = VkInstanceContext{};
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
        .pApplicationName = APP_NAME,
        .applicationVersion = vk::makeApiVersion(0, 0, 1, 0),
        .pEngineName = "No Engine",
        .engineVersion = vk::makeApiVersion(0, 0, 1, 0),
        .apiVersion = API_VER,
    };

    using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;
    auto const instanceDebugInfo = vk::DebugUtilsMessengerCreateInfoEXT{
        .messageSeverity = Severity::eWarning | Severity::eError,
        .messageType = Type::eGeneral | Type::eValidation | Type::ePerformance,
        .pfnUserCallback = vk_debug_callback,
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

void VkEngine::init_vma(){
    auto vma_vulkan_functions = VmaVulkanFunctions{
        .vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr )SDL_Vulkan_GetVkGetInstanceProcAddr(),
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
    };
    // BUG: VMA REPORTED:
    //Assertion failed: 
    // (pCreateInfo->physicalDevice && pCreateInfo->device && pCreateInfo->instance), function VmaAllocator_T, file vk_mem_alloc.h, line 13347.
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
}
void VkEngine::cleanup_vma()const noexcept{
    vmaDestroyAllocator(m_allocator);
}
void VkEngine::init_vk_instance(){
    auto ctx = make_vk_instance(m_useValidationLayers, m_vkContext, API_VER);
    m_vkInstance = std::move(ctx.m_vkInstance);
    m_vkDebugMessenger = std::move(ctx.m_vkDebugMessenger);
}

void VkEngine::init_descriptor_sets() {
    std::array<vk::DescriptorSetLayout, syncFrameCount> layouts{}; 
    layouts.fill(m_vkDescriptorSetLayout);
    ASSERT(layouts.size() == syncFrameCount);

    m_vkDescriptorSets = m_vkDevice.allocateDescriptorSets(
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
            .setSampler(m_vkTextureSampler)
            .setImageView(m_vkTextureImageView)
            .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
         ;
        auto descriptorWriteSets = std::array{
            vk::WriteDescriptorSet{}
                .setDstSet(*m_vkDescriptorSets[frame_idx])
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setBufferInfo(bufferInfo)
            ,
            vk::WriteDescriptorSet{}
                .setDstSet(*m_vkDescriptorSets[frame_idx])
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
}
[[nodiscard]]
vk::raii::DescriptorPool make_descriptor_pool(
    vk::raii::Device const& m_vkDevice,
    u32 syncFrameCount
){
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
void VkEngine::init_descriptor_pool() {
    m_vkDescriptorPool = make_descriptor_pool(m_vkDevice, syncFrameCount);
}

[[nodiscard]]
vk::raii::DescriptorSetLayout make_descriptor_set_layout(
    vk::raii::Device const& m_vkDevice
) {
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
void VkEngine::init_descriptor_set_layout() {
    m_vkDescriptorSetLayout = make_descriptor_set_layout(m_vkDevice);
}


[[nodiscard]] auto make_shader_module(vk::raii::Device const& m_vkDevice, std::span<const char> spirv_src){
    ASSERT(spirv_src.size() == spirv_src.size_bytes());
    return vk::raii::ShaderModule{
        m_vkDevice,
        vk::ShaderModuleCreateInfo{
            .codeSize = spirv_src.size(),
            .pCode = reinterpret_cast<u32 const*>(spirv_src.data()),
        },
    };
}
void VkEngine::init_pipeline() {
    auto shader_src = read_file_contents("shaders/slang.spv");
    auto shader_module = make_shader_module(m_vkDevice, shader_src);

    auto vtxStageInfo = vk::PipelineShaderStageCreateInfo{}
        .setStage(vk::ShaderStageFlagBits::eVertex)
        .setModule(shader_module)
        .setPName("vertMain")
    ;
    auto fragStageInfo = vk::PipelineShaderStageCreateInfo{}
        .setStage(vk::ShaderStageFlagBits::eFragment)
        .setModule(shader_module)
        .setPName("fragMain")
    ;
    std::array shaderStages{
        vtxStageInfo,
        fragStageInfo,
    };
    auto dynamicState = vk::PipelineDynamicStateCreateInfo{}
        .setDynamicStates(vk_enabledDynamicState);

    auto vertexInputState = vk::PipelineVertexInputStateCreateInfo{}
        .setVertexBindingDescriptions(VertexTraits<Vertex>::binding_desc)
        .setVertexAttributeDescriptions(VertexTraits<Vertex>::attribute_desc);


    auto const inputAssemblyState=  vk::PipelineInputAssemblyStateCreateInfo{
        .topology = vk::PrimitiveTopology::eTriangleList,
    };

    auto viewportState = vk::PipelineViewportStateCreateInfo{
        .viewportCount = 1,
        .scissorCount = 1,
        // In this instance, since these are both dynamic state, 
        // we dont set them here, but rather during the draw call or smth
    };
    auto rasterizationState = vk::PipelineRasterizationStateCreateInfo{
        .depthClampEnable        = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode             = vk::PolygonMode::eFill,
        .cullMode                = vk::CullModeFlagBits::eNone,
        .frontFace               = vk::FrontFace::eClockwise,
        .depthBiasEnable         = vk::False,
        .lineWidth               = 1.0f
    };
    auto multisamplingState = vk::PipelineMultisampleStateCreateInfo{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False,
    };

    // regular alpha blending
    auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState{
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
    auto colorBlendState = vk::PipelineColorBlendStateCreateInfo{
        .logicOpEnable = false,
        .logicOp = vk::LogicOp::eCopy,
    }.setAttachments(colorBlendAttachment);

    m_vkPipelineLayout = vk::raii::PipelineLayout{
        m_vkDevice,
        vk::PipelineLayoutCreateInfo{}
            .setPushConstantRangeCount(0)
            .setSetLayouts(*m_vkDescriptorSetLayout)
    };

    auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo{}
        .setDepthTestEnable(vk::True)
        .setDepthWriteEnable(vk::True)
        .setDepthCompareOp(vk::CompareOp::eLess)
        .setDepthBoundsTestEnable(vk::False)
        .setStencilTestEnable(vk::False)
    ;

    auto pipelineCreateInfoChain =  vk::StructureChain{
        vk::GraphicsPipelineCreateInfo{}
            .setStageCount(shaderStages.size())
            .setPStages(shaderStages.data())
            .setPVertexInputState(&vertexInputState)
            .setPInputAssemblyState(&inputAssemblyState)
            .setPViewportState(&viewportState)
            .setPRasterizationState(&rasterizationState)
            .setPMultisampleState(&multisamplingState)
            .setPColorBlendState(&colorBlendState)
            .setPDynamicState(&dynamicState)
            .setLayout(m_vkPipelineLayout)
            .setPDepthStencilState(&depthStencilState)
            .setRenderPass(nullptr)
        ,
        vk::PipelineRenderingCreateInfo{}
            .setColorAttachmentCount(1)
            .setPColorAttachmentFormats( &m_swapchain.imageFormat)
            .setDepthAttachmentFormat(select_depth_format())
    };
    m_vkPipeline = vk::raii::Pipeline(
        m_vkDevice,
        nullptr,
        pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>()
    );
}

[[nodiscard]]
std::vector<FrameData> make_inflightFrames(
    vk::raii::Device const& m_vkDevice,
    u32 frameCount,
    u32 m_vkQueueFamily
) {
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
    }

    return m_inflightFrames;
}
void VkEngine::init_inflightFrames() {
    m_inflightFrames = make_inflightFrames(m_vkDevice,syncFrameCount, m_vkQueueFamily);
}

void VkEngine::init_sync_structures() {
    int frame_idx = 0;
    for (auto& frame : m_inflightFrames) {
        // Fences are cpu<->gpu. 
        frame.fence = vk::raii::Fence{
            m_vkDevice,
            vk::FenceCreateInfo{
                .flags = vk::FenceCreateFlagBits::eSignaled,
            },
        };
        set_vk_dbg_name(m_vkDevice, frame.fence, std::format("(Frame {}) fence", frame_idx));

        frame.presentCompleteSemaphore = vk::raii::Semaphore{
            m_vkDevice,
            vk::SemaphoreCreateInfo{ }
        };
        set_vk_dbg_name(m_vkDevice, frame.presentCompleteSemaphore, std::format("(Frame {}) present-complete sem", frame_idx));
        frame_idx++;
    }
}

struct ImageData{
    static_assert(sizeof(unsigned char) == sizeof(std::byte));
    static constexpr i32 img_bytes_per_channel {1}; 
    static constexpr i32 desired_channels {STBI_rgb_alpha}; 
    static constexpr auto vk_format = vk::Format::eR8G8B8A8Srgb;

    constexpr u32 size_bytes(){
        return img_bytes_per_channel * px_w * px_h * desired_channels;
    }
    constexpr vk::Extent2D get_extent2d(){
        return vk::Extent2D{
            static_cast<u32>(px_w),
            static_cast<u32>(px_h),
        };
    }
    i32 px_w{}, px_h{}, n_src_channels{};
    std::span<std::byte> data;
    constexpr void free_buffer(){
        stbi_image_free(data.data());
    }
};
ImageData load_from_filename(std::string filename){
    ImageData img{};
    auto* img_data = stbi_load(filename.c_str(), &img.px_w,&img.px_h,&img.n_src_channels,img.desired_channels);
    img.data = std::span(reinterpret_cast<std::byte*>(img_data), img.size_bytes());
    if (!img_data){
        LOG_FATAL(
            "Unable to load image at '{}'. reason:{}",
            filename,
            stbi_failure_reason()
        );
    }
    return img;
}

void VkEngine::init_textures() {
    auto img = load_from_filename("./textures/tim_cheese.png");

    auto texStagingBuf = GPUBuffer::make_staging(m_vkDevice,m_vkPhysicalDevice, img.size_bytes());
    texStagingBuf.upload_data(img.data);
    img.free_buffer();

    m_vkTextureImage = vk::raii::Image{
        m_vkDevice,
        vk::ImageCreateInfo{}
            .setImageType(vk::ImageType::e2D)
            .setFormat(img.vk_format)
            .setExtent({static_cast<u32>(img.px_w),static_cast<u32>(img.px_h),1})
            .setMipLevels(1)
            .setArrayLayers(1)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
            .setSharingMode(vk::SharingMode::eExclusive)
    };
    set_vk_dbg_name(m_vkDevice,m_vkTextureImage, "Texture image");

    auto memRequirements = m_vkTextureImage.getMemoryRequirements();
    m_vkTextureImageMemory = vk::raii::DeviceMemory{
        m_vkDevice,
        vk::MemoryAllocateInfo{}
            .setAllocationSize(memRequirements.size)
            .setMemoryTypeIndex(
                select_memory_type(
                    m_vkPhysicalDevice,
                    memRequirements.memoryTypeBits,
                    vk::MemoryPropertyFlagBits::eDeviceLocal
                )
            )
    };
    set_vk_dbg_name(m_vkDevice,m_vkTextureImageMemory, "Texture image memory");
    m_vkTextureImage.bindMemory(m_vkTextureImageMemory,0);

    auto cmdBuf = begin_single_use_cmd();
    set_vk_dbg_name(m_vkDevice,cmdBuf, "Texture image transition layout buffer");

    transition_img_layout(
        cmdBuf,
        m_vkTextureImage, 
        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
        vk::ImageAspectFlagBits::eColor
    );
    copy_buffer_to_image(cmdBuf,texStagingBuf.buf, m_vkTextureImage, img.get_extent2d());
    transition_img_layout(
        cmdBuf,
        m_vkTextureImage, 
        vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ImageAspectFlagBits::eColor
    );
    end_single_use_cmd(std::move(cmdBuf));


    m_vkTextureImageView = vk::raii::ImageView(
        m_vkDevice,
        vk::ImageViewCreateInfo{}
            .setImage(m_vkTextureImage)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(img.vk_format)
            .setSubresourceRange(
                vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1)
                    .setLevelCount(1)
            )
    );
    set_vk_dbg_name(m_vkDevice,m_vkTextureImageView, "Texture imageview");


    m_vkTextureSampler = vk::raii::Sampler(
        m_vkDevice,
        vk::SamplerCreateInfo{}
            .setMagFilter(vk::Filter::eNearest)
            .setMinFilter(vk::Filter::eNearest)
            .setMipmapMode(vk::SamplerMipmapMode::eNearest)
            .setMipLodBias(0.0f)
            .setMinLod(0.0f)
            .setMaxLod(0.0f)
            .setAddressModeU(vk::SamplerAddressMode::eRepeat)
            .setAddressModeV(vk::SamplerAddressMode::eRepeat)
            .setAddressModeW(vk::SamplerAddressMode::eRepeat)
            .setAnisotropyEnable(vk::True)
            .setMaxAnisotropy(m_vkPhysicalDevice.getProperties().limits.maxSamplerAnisotropy)
            .setCompareEnable(vk::False)
            .setCompareOp(vk::CompareOp::eAlways)
            .setBorderColor(vk::BorderColor::eIntOpaqueWhite)
            .setUnnormalizedCoordinates(vk::False)
            
    );
    set_vk_dbg_name(m_vkDevice,m_vkTextureSampler, "Texture sampler");

}

void VkEngine::init_depth_attachment() {
    m_vkDepthImage= vk::raii::Image{
        m_vkDevice,
        vk::ImageCreateInfo{}
            .setImageType(vk::ImageType::e2D)
            .setFormat(select_depth_format())
            .setExtent({m_swapchain.extent.width,m_swapchain.extent.width,1})
            .setMipLevels(1)
            .setArrayLayers(1)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
            .setSharingMode(vk::SharingMode::eExclusive)
    };
    set_vk_dbg_name(m_vkDevice,m_vkDepthImage, "Depth image");

    auto memRequirements = m_vkDepthImage.getMemoryRequirements();
    m_vkDepthImageMemory = vk::raii::DeviceMemory{
        m_vkDevice,
        vk::MemoryAllocateInfo{}
            .setAllocationSize(memRequirements.size)
            .setMemoryTypeIndex(
                select_memory_type(
                    m_vkPhysicalDevice,
                    memRequirements.memoryTypeBits,
                    vk::MemoryPropertyFlagBits::eDeviceLocal
                )
            )
    };
    set_vk_dbg_name(m_vkDevice,m_vkDepthImageMemory, "Depth image memory");
    m_vkDepthImage.bindMemory(m_vkDepthImageMemory,0);

    m_vkDepthImageView = vk::raii::ImageView(
        m_vkDevice,
        vk::ImageViewCreateInfo{}
            .setImage(m_vkDepthImage)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(select_depth_format())
            .setSubresourceRange(
                vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eDepth)
                    .setBaseMipLevel(0)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1)
                    .setLevelCount(1)
            )
    );
    
}
