#include <thread>


#include "sdl3_types.hpp"
#include "glm_types.hpp"

#include "format_specs.hpp"
#include "file_io.hpp"
#include "shared.hpp"
#include "timer.hpp"
#include "vertex.hpp"
#include "vk_engine.hpp"
#include "vertex_raw_data.hpp"
#include "vk_debug.hpp"
#include "vk_util.hpp"
#include "vk_buffers.hpp"
#include "vk_init_helpers.hpp"

static char const* APP_NAME = "Test Window";

void VkEngine::init() {
    LOG_INFO("INITIALIZING ENGINE ({})", static_cast<void*>(this));
    ASSERT(m_loadedEngine == nullptr);
    m_loadedEngine = this;
    try{
        init_window();
        init_vk_instance();
        init_vk_surface();
        init_vk_device_and_queue();
        init_swapchain();
        init_inflightFrames();
        init_descriptor_set_layout();
        init_buffers();  //  TODO: refactor to be like the others
        init_pipeline();  //  TODO: refactor to be like the others
        init_descriptor_pool();
        init_descriptor_sets(); //  TODO: refactor to be like the others
        init_sync_structures(); //  TODO: refactor to be like the others
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
    return m_window;
}

void VkEngine::init_window() {
    m_window = make_window(m_windowLogicalSize);
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
    for (auto const& image: swapchain.images){
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
    // VERTEX BUFFER
    auto const& vtx_src = std::span{vtx_raw_data::ccw_quad_verts};
    auto [
        vtxStagingBuf,
        vtxStagingMem
    ] = make_staging_buffer(vtx_src.size_bytes());
    void *vtx_staging_data = vtxStagingMem.mapMemory(0,vtx_src.size_bytes());
    std::memcpy(vtx_staging_data, vtx_src.data(), vtx_src.size_bytes());
    vtxStagingMem.unmapMemory();
    

    std::tie(
        m_vertexBuffer,
        m_vertexBufferMemory
    ) = make_vertex_buffer(vtx_src.size_bytes(), vk::SharingMode::eExclusive);
    copy_buffer(vtxStagingBuf,m_vertexBuffer,vtx_src.size_bytes());
    set_vk_dbg_name(m_vkDevice, m_vertexBuffer, "Vertex buffer");



    // INDEX BUFFER
    auto const& idx_src = std::span{vtx_raw_data::ccw_quad_indices};

    auto [
        idxStagingBuf, 
        idxStagingMem
    ] = make_staging_buffer(idx_src.size_bytes());
    void *idx_staging_data = idxStagingMem.mapMemory(0,idx_src.size_bytes());
    std::memcpy(idx_staging_data, idx_src.data(), idx_src.size_bytes());
    idxStagingMem.unmapMemory();

    std::tie(
        m_indexBuffer,
        m_indexBufferMemory
    ) = make_index_buffer(idx_src.size_bytes(), vk::SharingMode::eExclusive);

    copy_buffer(idxStagingBuf,m_indexBuffer,idx_src.size_bytes());
    set_vk_dbg_name(m_vkDevice, m_vertexBuffer, "Index buffer");


    // UNIFORM BUFFERS
    for (auto& frame : m_inflightFrames){
        auto const bufSize = static_cast<u32>(sizeof(UniformBufferObject));
        std::tie(
            frame.uniformBuffer,
            frame.uniformBufferMemory 
        ) = make_uniform_buffer(bufSize,vk::SharingMode::eExclusive);
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
void VkEngine::init_vk_instance(){
    auto ctx = make_vk_instance(m_useValidationLayers, m_vkContext, API_VER);
    m_vkInstance = std::move(ctx.m_vkInstance);
    m_vkDebugMessenger = std::move(ctx.m_vkDebugMessenger);
}

[[nodiscard]] auto VkEngine::make_shader_module(std::span<const char> spirv_src){
    ASSERT(spirv_src.size() == spirv_src.size_bytes());
    return vk::raii::ShaderModule{
        m_vkDevice,
        vk::ShaderModuleCreateInfo{
            .codeSize = spirv_src.size(),
            .pCode = reinterpret_cast<u32 const*>(spirv_src.data()),
        },
    };
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
    for (auto i = 0uz; i<syncFrameCount; i++){
        auto bufferInfo = 
            vk::DescriptorBufferInfo{}
            .setBuffer(*m_inflightFrames[i].uniformBuffer)
            .setOffset(0)
            .setRange(sizeof(UniformBufferObject))
         ;
        m_vkDevice.updateDescriptorSets(
            vk::WriteDescriptorSet{}
                .setDstSet(*m_vkDescriptorSets[i])
                .setDstBinding(0)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setBufferInfo(bufferInfo),
             {}
        );
    }
}
[[nodiscard]]
vk::raii::DescriptorPool make_descriptor_pool(
    vk::raii::Device const& m_vkDevice,
    u32 syncFrameCount
){
    auto poolSizes = vk::DescriptorPoolSize{}
        .setType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(syncFrameCount)
    ;
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
    auto uboLayoutBinding = vk::DescriptorSetLayoutBinding{}
        .setBinding(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eVertex)
    ;

    return vk::raii::DescriptorSetLayout{
        m_vkDevice, 
        vk::DescriptorSetLayoutCreateInfo{}
            .setBindingCount(1)
            .setBindings(uboLayoutBinding)
    };
}
void VkEngine::init_descriptor_set_layout() {
    m_vkDescriptorSetLayout = make_descriptor_set_layout(m_vkDevice);
}


void VkEngine::init_pipeline() {
    auto shader_src = read_file_contents("shaders/slang.spv");
    auto shader_module = make_shader_module(shader_src);

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


    auto const iaState=  vk::PipelineInputAssemblyStateCreateInfo{
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

    auto pipelineCreateInfoChain =  vk::StructureChain{
        vk::GraphicsPipelineCreateInfo{
            .stageCount = shaderStages.size(),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputState,
            .pInputAssemblyState = &iaState,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizationState,
            .pMultisampleState = &multisamplingState,
            .pColorBlendState = &colorBlendState,
            .pDynamicState = &dynamicState,
            .layout = m_vkPipelineLayout,
            .renderPass = nullptr,
        },
        vk::PipelineRenderingCreateInfo{
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &m_swapchain.imageFormat, 
        },
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

void VkEngine::cleanup() {
    if (is_initialized()) {
        m_vkDevice.waitIdle();

        m_swapchain = Swapchain{};
        for (auto& frame: m_inflightFrames){
            frame = FrameData{};
        }
        m_vkQueue.clear();
        m_vkDescriptorSetLayout.clear();
        m_vkPipelineLayout.clear();
        m_vkPipeline.clear();

        m_vertexBuffer.clear();
        m_vertexBufferMemory.clear();

        m_vkDescriptorSets.clear();
        m_vkDescriptorPool.clear();

        m_indexBuffer.clear();
        m_indexBufferMemory.clear();

        m_vkDevice.clear();
        m_vkPhysicalDevice.clear();
        m_vkSurface.clear();
        // this cant be done here, shouldnt it happen after destruction of raii stuff?
        m_window = nullptr;
    }
    m_loadedEngine = nullptr;
}


void VkEngine::cleanup_window() const noexcept{
    SDL_DestroyWindow(m_window);
}

