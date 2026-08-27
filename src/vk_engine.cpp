

#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL.h>
#include <thread>

#include "vk_engine.hpp"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_stdinc.h"
#include "SDL3/SDL_video.h"
#include "VkBootstrap.h"
#include "file_io.hpp"
#include "vk_debug.hpp"
#include "shared.hpp"
#include "vk_util.hpp"
#include "vk_init_helpers.hpp"
#include "vulkan/vulkan.hpp"

static char const* APP_NAME = "Test Window";

void VkEngine::recreate_swapchain() {
    using namespace std::chrono_literals;
    auto framebuffer_sz = get_framebuffer_size();
    while (framebuffer_sz.width == 0 || framebuffer_sz.height == 0){
        // window is minimized 
        std::this_thread::sleep_for(10ms);
        handle_inputs();
        framebuffer_sz = get_framebuffer_size();
    }
    m_vkDevice.waitIdle();
    m_swapchain.descriptor.clear();
    init_swapchain();
}

void VkEngine::init_window() {
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
}

void VkEngine::init_swapchain() {
    m_swapchain = Swapchain::make(
        SwapchainSettings{
            .physical_device = m_vkPhysicalDevice,
            .device = m_vkDevice,
            .surface = m_vkSurface,
            .extent_px = get_framebuffer_size(),
        }
    );
    int idx = 0;
    for (const auto& rend_sem: m_swapchain.renderFinishedSemaphores){
        setObjectName(rend_sem, std::format("({}) render sem", idx));
        idx++;
    }
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
    int frame_idx = 0;
    for (auto& frame : m_inflightFrames) {
        // Fences are cpu<->gpu. 
        frame.fence = vk::raii::Fence{
            m_vkDevice,
            vk::FenceCreateInfo{
                .flags = vk::FenceCreateFlagBits::eSignaled,
            },
        };
        setObjectName(frame.fence, std::format("(Frame {}) fence", frame_idx));

        frame.presentCompleteSemaphore = vk::raii::Semaphore{
            m_vkDevice,
            vk::SemaphoreCreateInfo{ }
        };
        setObjectName(frame.presentCompleteSemaphore, std::format("(Frame {}) present-complete sem", frame_idx));
        frame_idx++;
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

    // 2. Acquire the next image from the swapchain
    // BUG: We are reaching here, with the presentComplete semaphore ALREADY SIGNALLED.
    auto [acquire_res, imageIndex] = m_swapchain.descriptor.acquireNextImage(
        numeric_max<u64>,
        *frame.presentCompleteSemaphore
    );
    if (acquire_res == vk::Result::eErrorOutOfDateKHR){
        // only early return if we got no image at all, and thus didnt signal the semaphore.
        // Else we get validation error on the semaphore
        recreate_swapchain();
        return;
    }else{
        ASSERT(acquire_res==vk::Result::eSuccess || acquire_res==vk::Result::eSuboptimalKHR);
    }
    // only reset if we acquired an image from the swapchain, else we early returned
    m_vkDevice.resetFences(*frame.fence);

    auto & cmdBuf = frame.commandBuffer;
    cmdBuf.reset();
    record_commands_to_buffer(imageIndex);

    auto submit_info = vk::SubmitInfo{};
    vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    submit_info.setCommandBuffers(*cmdBuf);
    submit_info.setWaitSemaphores(*frame.presentCompleteSemaphore);
    submit_info.setSignalSemaphores(*m_swapchain.renderFinishedSemaphores[imageIndex]);
    submit_info.setWaitDstStageMask(waitDstStageMask);
    m_vkQueue.submit(
        submit_info,
        *frame.fence
    );

    auto const present_res = m_vkQueue.presentKHR(
        vk::PresentInfoKHR{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &*m_swapchain.renderFinishedSemaphores[imageIndex],

            .swapchainCount = 1,

            .pSwapchains = &*m_swapchain.descriptor,

            .pImageIndices = &imageIndex,
        }
    );
    if (   present_res == vk::Result::eSuboptimalKHR 
        || present_res == vk::Result::eErrorOutOfDateKHR
        || m_framebufferResized
    ){
        m_framebufferResized = false;
        recreate_swapchain();
    }else{
        ASSERT(present_res==vk::Result::eSuccess);
    }

    m_frameCount++;
}

void VkEngine::handle_inputs(){
    SDL_Event e{};
    while ((SDL_PollEvent(&e)) != 0) {
        if (e.type == SDL_EVENT_QUIT) {
            m_shouldStopRunning= true;
        }
        if (e.type == SDL_EVENT_WINDOW_MINIMIZED) {
            m_shouldStopRendering = true;
        }
        if (e.type == SDL_EVENT_WINDOW_RESTORED) {
            m_shouldStopRendering = false;
        }
        if (e.type == SDL_EVENT_WINDOW_RESIZED) {
            m_framebufferResized = true;
        }
    }
}

void VkEngine::run() {
    using namespace std::chrono_literals;
    while (!m_shouldStopRunning) {
        handle_inputs();
        if (m_shouldStopRendering) {
            std::this_thread::sleep_for(100ms);
            continue;
        }
        draw();
        std::println("fps:{:4.2f}",
                     m_frameCount / timer::get_seconds(timer::since_epoch()));
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
        m_vkPipelineLayout.clear();
        m_vkPipeline.clear();
        m_vkDevice.clear();
        m_vkPhysicalDevice.clear();
        m_vkSurface.clear();
        // this cant be done here, shouldnt it happen after destruction of raii stuff?
        m_window = nullptr;
    }
    m_loadedEngine = nullptr;
}

VkEngine& VkEngine::get_instance() { 
    return *m_loadedEngine; 
}
bool VkEngine::is_initialized() { 
    return m_window; 
}

FrameData& VkEngine::get_current_frame() {
    return m_inflightFrames.at(m_frameCount % syncFrameCount);
}

void VkEngine::cleanup_window() const noexcept{
    SDL_DestroyWindow(m_window);
}

[[nodiscard]]
vk::Extent2D VkEngine::get_framebuffer_size() const noexcept{
    int w{},h{};
    SDL_GetWindowSizeInPixels(m_window, &w,&h);
    return vk::Extent2D{st_cast<u32>(w),st_cast<u32>(h)};
}
