#include <cpptrace/basic.hpp>
#include <vulkan/vulkan.hpp>
#include "vk_types.hpp"
#include "range/v3/view/enumerate.hpp"
// stdlib
#include <format>
#include <print>
#include <thread>
#include <type_traits>

// Libraries
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cpptrace/utils.hpp>
#include <cpptrace/formatting.hpp>
#include <SDL3/SDL_video.h>
#include <VkBootstrap.h>
#include <vulkan/vulkan_raii.hpp>

// Project headers
#include "types.hpp"
#include "vk_init_helpers.hpp"
#include "file_io.hpp"
#include "vk_images.hpp"
#include "vk_swapchain.hpp"
#include "logger.hpp"
#include "shared.hpp"
#include "timer.hpp"
#include "vulkan/vulkan.hpp"
//#include "vulkan/vulkan.hpp"


char const* APP_NAME = "Test Window";

struct FrameData {
    vk::raii::CommandPool commandPool{nullptr};
    vk::raii::CommandBuffer commandBuffer{nullptr};
    vk::raii::Fence fence{nullptr};
    vk::raii::Semaphore presentCompleteSemaphore{nullptr};
};

FWD_DECL_STRUCT(SDL_Window);
struct VkEngine {
  public:
    template <typename T, size_t S> using array = std::array<T, S>;
    static constexpr u32 N_FSYNC = 2;
    VkEngine() { init(); }
    ~VkEngine() { cleanup(); }
    VkEngine* m_loadedEngine{};

    size_t m_frameCount{};
    bool m_shouldStopRendering{};
    SDL_Window* m_window{};
    static constexpr vk::Extent2D m_windowExtent{800, 600};
    static constexpr bool m_useValidationLayers{true};

    vk::raii::Context m_vkContext{};
    vk::raii::Instance m_vkInstance{nullptr};
    vk::raii::DebugUtilsMessengerEXT m_vkDebugMessenger{nullptr};
    vk::raii::SurfaceKHR m_vkSurface{nullptr};
    vk::raii::PhysicalDevice m_vkPhysicalDevice{nullptr};
    vk::raii::Device m_vkDevice{nullptr};
    vk::raii::Queue m_vkQueue{nullptr};
    u32 m_vkQueueFamily{};


    Swapchain m_swapchain{};

    array<FrameData, N_FSYNC> m_inflightFrames{};

    vk::raii::PipelineLayout m_vkPipelineLayout{nullptr};
    vk::raii::Pipeline m_vkPipeline{nullptr};

    vk::raii::CommandPool m_commandPool{nullptr};
    void record_commands_to_buffer(u32 imageIndex);

    FrameData& get_current_frame();


    VkEngine& get();

    bool is_initialized();

    void init();
    void cleanup();

    void run();
    void draw();

  private:
    void init_window();
    void init_vulkan();
    void init_swapchain();
    void init_pipeline();

    [[nodiscard]] 
    auto make_shader_module(std::span<const char> spirv_src);
    void init_commands();
    void init_sync_structures();

    void cleanup_swapchain();
};

void VkEngine::init_window() {
    SDL_Init(SDL_INIT_VIDEO);
    m_window = SDL_CreateWindow(APP_NAME, m_windowExtent.width,
                                m_windowExtent.height, SDL_WINDOW_VULKAN);
    if (!m_window) {
        LOG_ERROR("Failed to init window");
        LOG_EXIT(1);
    }
}
VKAPI_ATTR vk::Bool32 VKAPI_CALL vk_debug_callback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT types,
    vk::DebugUtilsMessengerCallbackDataEXT const* data,
    void* _
) {
    auto custom_formatter = cpptrace::formatter{}
        .colors(cpptrace::formatter::color_mode::always)    // Force ANSI colors
        .paths(cpptrace::formatter::path_mode::basename)    // Print "main.cpp" instead of full path
        .addresses(cpptrace::formatter::address_mode::none) // Object file address instead of raw
        .snippets(false);                                    // Include source code snippets
    using Sev = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    auto const kind = vk::to_string(types);
    if (severity >= Sev::eError) {
        std::ostringstream out{};
        auto trace = cpptrace::generate_trace();
        for (auto idx = 0uz; idx< trace.frames.size(); idx++){
            auto const& frame = trace.frames.at(idx);
            bool has_dbg_symbols = frame.line.has_value();
            if (has_dbg_symbols){
                out << custom_formatter.format(frame) << "\n";
            }else{
                auto omitted_streak = 0uz;
                while (idx < trace.frames.size() && !trace.frames.at(idx).line.has_value()){
                    omitted_streak++;
                    idx++;
                }
                out << std::format("Omitted {} frames (no dbg_symbols)\n",omitted_streak);
            }
        }
        std::cerr << out.str();
        LIBASSERT_BREAKPOINT();
    }
    return vk::False;
}

void VkEngine::init_vulkan() try {
    static constexpr auto API_VER = vk::ApiVersion13;
    constexpr auto EXT_MOLTENVK_FIX = "VK_KHR_portability_subset";
    constexpr auto VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";
    // sdl requires some extensions for its windowing stuff
    
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

    m_vkInstance = vk::raii::Instance{
        m_vkContext,
        vk::InstanceCreateInfo{
            // without this pnext, vulkan cannot report errors to the debug extension during instance creation
            .pNext = m_useValidationLayers ? &instanceDebugInfo : nullptr,
            .flags = instanceFlags,
            .pApplicationInfo = &appInfo,

            .enabledLayerCount = static_cast<u32>(appLayers.size()),
            .ppEnabledLayerNames = appLayers.data(),

            .enabledExtensionCount = static_cast<u32>(instanceExtensions.size()),
            .ppEnabledExtensionNames = instanceExtensions.data(),

        },
    };

    if (m_useValidationLayers){
        m_vkDebugMessenger = vk::raii::DebugUtilsMessengerEXT{m_vkInstance, instanceDebugInfo};
    }
    ASSERT(m_window);

    // Have SDL create the surface given our instance we just setup 
    auto raw_surface = VkSurfaceKHR{};
    if (!SDL_Vulkan_CreateSurface(m_window, get_c_handle(m_vkInstance), nullptr,
                                  &raw_surface)) {
        LOG_FATAL("Failure in {}(): {}", "SDL_Vulkan_CreateSurface", SDL_GetError());
    }
    m_vkSurface = vk::raii::SurfaceKHR{m_vkInstance, raw_surface};

    // NOTE: Instance extensions modify global behaviour BEFORE a device is selected,
    // whereas DEVICE extensions modify the behaviour of a specific vk::Device.
    // Configure Device extensions
    auto device_extensions = std::vector<char const*>{};

    // iterate over all physical devices listed by driver
    for (auto const& physical_device : m_vkInstance.enumeratePhysicalDevices()){
        // skip if device doesnt support expected api version
        auto device_api_ver = physical_device.getProperties().apiVersion;
        if (device_api_ver < API_VER) continue;

        // skip if device doesnt support khr swapchain
        auto const supported_extensions = physical_device.enumerateDeviceExtensionProperties();
        if (!supports_extension(supported_extensions,vk::KHRSwapchainExtensionName)){
            continue;
        }
        device_extensions.push_back(vk::KHRSwapchainExtensionName); 

        auto family = get_pd_queue_family(
            physical_device, 
            [physical_device, this](u32 idx){
                return physical_device.getSurfaceSupportKHR(idx,m_vkSurface) == vk::True;
            }
        );
        if (!family) continue; // no matching queue family on the device
        

        // 'VK_KHR_portability_subset must be enabled if physical device supports it.'
        if (supports_extension(supported_extensions,EXT_MOLTENVK_FIX)){
            device_extensions.push_back(EXT_MOLTENVK_FIX);
        }
        auto queue_prio = 1.0f;
        auto queue_info = vk::DeviceQueueCreateInfo{
            .queueFamilyIndex = m_vkQueueFamily,
            .queueCount = 1,
        };
        queue_info.setQueuePriorities(queue_prio);

        
        auto enabled_features = required_pd_feature_list();
        try {
            auto deviceCreateInfo= vk::DeviceCreateInfo{
                .pNext = &enabled_features.get<vk::PhysicalDeviceFeatures2>(),
            };
            deviceCreateInfo.setQueueCreateInfos(queue_info);
            deviceCreateInfo.setPEnabledExtensionNames(device_extensions);
            m_vkDevice = vk::raii::Device{
                physical_device,
                deviceCreateInfo,
            };
        } catch (vk::FeatureNotPresentError e){
            LOG_FATAL("PD {} is missing a feature: {}",get_pd_name(physical_device),e.what());
            continue;
        }

        m_vkPhysicalDevice = std::move(physical_device);
        m_vkQueueFamily = *family;
        break;
    }
    if (!*m_vkPhysicalDevice){
        LOG_FATAL("Unable to select a physical device! Driver listed {}, none matched",//
                  m_vkInstance.enumeratePhysicalDevices().size());
    }else{
        LOG_INFO("SELECTED GPU: {}",get_pd_name(m_vkPhysicalDevice));
    }

    m_vkQueue = m_vkDevice.getQueue(m_vkQueueFamily, 0);
} catch(vk::SystemError const& e){
    LOG_FATAL("Failed to initialize vulkan: {}", e.what());
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
void VkEngine::init_pipeline() {
    auto shader_src = read_file_contents("shaders/slang.spv");
    auto shader_module = make_shader_module(shader_src);

    auto vtxStageInfo = vk::PipelineShaderStageCreateInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shader_module,
        .pName = "vertMain",
    };
    auto fragStageInfo = vk::PipelineShaderStageCreateInfo{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shader_module,
        .pName = "fragMain",
    };
    std::array shaderStages{
        vtxStageInfo,
        fragStageInfo,
    };
    static constexpr std::array dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    auto dynamicState = vk::PipelineDynamicStateCreateInfo{ };
    dynamicState.setDynamicStates(dynamicStates);

    auto const vertexInputState = vk::PipelineVertexInputStateCreateInfo{};


    auto const iaState=  vk::PipelineInputAssemblyStateCreateInfo{
        .topology = vk::PrimitiveTopology::eTriangleList,
    };

    auto viewport = vk::Viewport{
        0.0f,0.0f,
        st_cast<f32>(m_swapchain.extent.width),
        st_cast<f32>(m_swapchain.extent.height),
        0.0f,1.0f,
    };
    auto scissor_region = vk::Rect2D{
        vk::Offset2D{},
        m_swapchain.extent,
    };

    auto viewportState = vk::PipelineViewportStateCreateInfo{
        .viewportCount = 1,
        .scissorCount = 1,
        // In this instance, since these are both dynamic state, we dont actually set the pointers to them

    };
    auto rasterizationState = vk::PipelineRasterizationStateCreateInfo{
        .depthClampEnable        = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode             = vk::PolygonMode::eFill,
        .cullMode                = vk::CullModeFlagBits::eBack,
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
    };
    colorBlendState.setAttachments(colorBlendAttachment);

    m_vkPipelineLayout = vk::raii::PipelineLayout{
        m_vkDevice,
        vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 0,
            .pushConstantRangeCount = 0,
        },
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
void VkEngine::init_swapchain() {
    m_swapchain = make_swapchain(
        SwapchainSettings{
            .physical_device = m_vkPhysicalDevice,
            .device = m_vkDevice,
            .surface = m_vkSurface,
            .extents = m_windowExtent,
        }
    );
}

void VkEngine::init_commands() {
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


}

void VkEngine::init_sync_structures() {
    for (auto& frame : m_inflightFrames) {
        // Fences are cpu<->gpu. 
        frame.fence = vk::raii::Fence{
            m_vkDevice,
            vk::FenceCreateInfo{
                .flags = vk::FenceCreateFlagBits::eSignaled,
            },
        };

        frame.presentCompleteSemaphore = vk::raii::Semaphore{
            m_vkDevice,
            vk::SemaphoreCreateInfo{}
        };
    }
}

void VkEngine::init() {
    LOG_INFO("INITIALIZING ENGINE ({})", static_cast<void*>(this));
    ASSERT(m_loadedEngine == nullptr);
    m_loadedEngine = this;
    init_window();
    init_vulkan();
    init_swapchain();
    init_pipeline();
    init_commands();
    init_sync_structures();
}
void VkEngine::record_commands_to_buffer(u32 imageIndex){
    auto& cmdBuf = get_current_frame().commandBuffer;
    cmdBuf.begin({});
    auto const swapchainImage = m_swapchain.images.at(imageIndex);

    vk_util::transition_image_layout(
        cmdBuf, swapchainImage, 
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,  
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );

    auto clearColor = vk::ClearColorValue{.float32 = {{
        std::abs(std::sin(m_frameCount / 120.0f)),
        0.0f,
        std::abs(std::sin(m_frameCount / 120.0f)),
        1.0f
        }}
    };

    auto attachmentInfo = vk::RenderingAttachmentInfo {
        .imageView = m_swapchain.imageViews.at(imageIndex),
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = {clearColor},
    };
    auto renderingInfo = vk::RenderingInfo{
        .renderArea{.offset={},.extent=m_swapchain.extent},
        .layerCount = 1,
    };
    renderingInfo.setColorAttachments(attachmentInfo);

    cmdBuf.beginRendering(renderingInfo);
    cmdBuf.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        *m_vkPipeline
    );
    // we specified viewport and scissor rect to be dynamic, thus we must set them before the draw cmd
    cmdBuf.setViewport(
        0, 
        vk::Viewport{
            0.0f,0.0f, // viewport position
            st_cast<f32>(m_swapchain.extent.width), st_cast<f32>(m_swapchain.extent.height),
            0.0f, 1.0f // min and max depth
        }
    );
    cmdBuf.setScissor(
        0,
        vk::Rect2D{
            vk::Offset2D{0,0},
            m_swapchain.extent
        }
    );
    u32 vertexCount{3};
    u32 instanceCount{1};
    u32 firstVertex{0};
    u32 firstInstance{0};
    cmdBuf.draw(vertexCount, instanceCount, firstVertex, firstInstance);
    cmdBuf.endRendering();
    vk_util::transition_image_layout(
        cmdBuf, swapchainImage, 
        vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {}, // must be eNone for ePresentSrcKHR
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,/* src stage mask*/
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );

    cmdBuf.end();
}

void VkEngine::draw() {
    auto& frame = get_current_frame();
    // 1. wait for previous frame to signal the fence
    auto fenceRV = m_vkDevice.waitForFences(*frame.fence, vk::True, numeric_max<u64>);
    if (fenceRV != vk::Result::eSuccess){
        LOG_FATAL("Failed to wait for fence");
    }
    m_vkDevice.resetFences(*frame.fence);

    // 2. Acquire the next image from the swapchain
    auto [res, imageIndex] = m_swapchain.descriptor.acquireNextImage(
        numeric_max<u64>,
        *frame.presentCompleteSemaphore
    );
    if (res == vk::Result::eSuboptimalKHR){
        LOG_WARN("Suboptimal swapchain acquire (wtv the fuck that means)");
    }

    auto & cmdBuf = frame.commandBuffer;
    cmdBuf.reset();
    record_commands_to_buffer(imageIndex);

    auto const cmdInfo = vk::CommandBufferSubmitInfo{
        .commandBuffer = *cmdBuf,
    };
    auto const waitInfo = vk::SemaphoreSubmitInfo{
        // Submission will wait until presentation of the previous frame is complete
        .semaphore = *frame.presentCompleteSemaphore,
        .value = 1,
        .stageMask = vk::PipelineStageFlagBits2::eAllCommands,
    };

    auto const signalInfo = vk::SemaphoreSubmitInfo{
        .semaphore = *m_swapchain.renderSemaphores[imageIndex],
        .value = 1,
        .stageMask = vk::PipelineStageFlagBits2::eAllCommands,
    };
    auto submit_info = vk::SubmitInfo2{};
    submit_info.setCommandBufferInfos(cmdInfo);
    submit_info.setWaitSemaphoreInfos(waitInfo);
    submit_info.setSignalSemaphoreInfos(signalInfo);
    m_vkQueue.submit2(
        submit_info,
        *frame.fence
    );
    try {
        auto const res = m_vkQueue.presentKHR(
            vk::PresentInfoKHR{
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &*m_swapchain.renderSemaphores[imageIndex],
                .swapchainCount = 1,
                .pSwapchains = &*m_swapchain.descriptor,
                .pImageIndices = &imageIndex,
            }
        );
        if (res == vk::Result::eSuboptimalKHR){
            // recreate swapchain
            PANIC("Unimplemented");
        }

    } catch(vk::OutOfDateKHRError const&){
        // recreate swapchain
        PANIC("Unimplemented");
    }
    m_frameCount++;
}
void VkEngine::run() {
    using namespace std::chrono_literals;
    SDL_Event e{};
    bool should_stop{false};
    while (!should_stop) {
        while ((SDL_PollEvent(&e)) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                should_stop = true;
            }
            if (e.type == SDL_EVENT_WINDOW_MINIMIZED) {
                m_shouldStopRendering = true;
            }
            if (e.type == SDL_EVENT_WINDOW_RESTORED) {
                m_shouldStopRendering = false;
            }
        }
        if (m_shouldStopRendering) {
            std::this_thread::sleep_for(100ms);
            continue;
        }
        draw();
        std::println("fps:{:4.2f}",
                     m_frameCount / timer::get_seconds(timer::since_epoch()));
    }
    // TODO: implement
}
void VkEngine::cleanup() {
    if (is_initialized()) {
        m_vkDevice.waitIdle();

        m_swapchain = Swapchain{};
        for (auto& frame: m_inflightFrames){
            frame = FrameData{};
        }
        m_vkQueue.clear();
        m_vkPipelineLayout.clear();
        m_vkPipeline.clear();
        m_vkDevice.clear();
        m_vkPhysicalDevice.clear();
        m_vkSurface.clear();
        // this cant be done here, shouldnt it happen after destruction of raii stuff?
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    m_loadedEngine = nullptr;
}
VkEngine& VkEngine::get() { return *m_loadedEngine; }
int main() {
    timer::set_prog_epoch();
    cpptrace::register_terminate_handler();
    {
        VkEngine engine{};
        engine.run();
    }
    LOG_EXIT(EXIT_SUCCESS);
}
bool VkEngine::is_initialized() { return m_window; }
FrameData& VkEngine::get_current_frame() {
    return m_inflightFrames.at(m_frameCount % N_FSYNC);
}
