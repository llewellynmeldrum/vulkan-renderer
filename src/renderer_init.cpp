#include <thread>
#include <span>
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
#include "renderer.hpp"
#include "vk_image_data.hpp"
#include "vertex_raw_data.hpp"


#include "vk_debug.hpp"
#include "vk_types.hpp"
#include "vk_util.hpp"
#include "vk_vertex_traits.hpp"
#include "vk_managed_buffers.hpp"
#include "renderer_init_helpers.hpp"
#include "renderer_buffer_helpers.hpp"

#include "renderer_init.hpp"
#include "renderer_types.hpp"


void Renderer::init(SDL_Window* window, glm::uvec2 window_extent) {
    m_pixelSize = SDL_GetWindowPixelDensity(window);
    m_window = window;
    m_windowLogicalExtent = {window_extent.x,window_extent.y};
    m_windowPixelExtent = logical_to_pixel(m_windowLogicalExtent);
    LOG_INFO("INITIALIZING RENDERER ({})", static_cast<void*>(this));
    init_vulkan();
}


void Renderer::init_vulkan() {
    //clang-format off
    try{
        {
            auto ctx = detail::make_vk_instance(k_useValidationLayers, m_vkContext, k_API_VER);
            m_vkInstance = std::move(ctx.m_vkInstance);
            m_vkDebugMessenger = std::move(ctx.m_vkDebugMessenger);
        }

        {
            m_vkSurface = detail::make_vk_surface(m_vkInstance, m_window);
        }

        {
            auto ctx = detail::make_vk_device_and_queue(m_vkInstance,m_vkSurface, k_API_VER);
            m_vkPhysicalDevice = std::move(ctx.m_vkPhysicalDevice);
            m_vkDevice         = std::move(ctx.m_vkDevice);
            m_vkQueue          = std::move(ctx.m_vkQueue);
            m_vkQueueFamily    = std::move(ctx.m_vkQueueFamily);
        }

        {
            m_allocator = detail::make_vma_allocator(m_vkPhysicalDevice, m_vkDevice, m_vkInstance);
        }

        {
            auto swap_settings = SwapchainSettings{
                .physical_device = m_vkPhysicalDevice,
                .device = m_vkDevice,
                .surface = m_vkSurface,
                .extent_px = get_framebuffer_size(),
            };
            m_swapchain = detail::make_swapchain(swap_settings);
        }

        {
            m_inflightFrames = detail::make_inflight_frames(
                m_vkDevice,
                m_vkPhysicalDevice,
                k_syncFrameCount,
                m_vkQueueFamily
            );
        }

        {
            init_texture(); 
        }

        {
            m_depthImage = DepthAttachment::make(
                m_vkDevice,
                m_allocator,
                m_swapchain.extent
            );
        }

        {
            m_vkDescriptorSetLayout = detail::make_descriptor_set_layout(m_vkDevice);
            m_vkDescriptorPool = detail::make_descriptor_pool(m_vkDevice, k_syncFrameCount);
            m_vkDescriptorSets = detail::make_descriptor_sets(
                m_vkDevice,
                m_vkDescriptorPool,
                m_vkDescriptorSetLayout,
                m_inflightFrames,
                m_texture,
                k_syncFrameCount
            );
        }


        {
            auto shader_src = read_file_contents(k_shader_spirv_path);
            auto shader_module = detail::helpers::make_shader_module(m_vkDevice, shader_src);
            init_fill_pipeline(shader_module);
            init_line_pipeline(shader_module);
        }

        {
            init_sync_structures();
        }
    }catch(vk::SystemError const& e){
        LOG_FATAL("Failed to initialize vulkan: {}", e.what());
    }
}




//void init_vma(
void Renderer::cleanup_vma()const noexcept{
    vmaDestroyAllocator(m_allocator);
}




template<typename VertexType>
auto make_shader_pipeline(ShaderPipelineCreateInfo info){

    auto vtxStageInfo = vk::PipelineShaderStageCreateInfo{}
        .setStage(vk::ShaderStageFlagBits::eVertex)
        .setModule(info.shader_module)
        .setPName(info.vertex_fn_name.c_str())
    ;
    auto fragStageInfo = vk::PipelineShaderStageCreateInfo{}
        .setStage(vk::ShaderStageFlagBits::eFragment)
        .setModule(info.shader_module)
        .setPName(info.frag_fn_name.c_str())
    ;
    std::array shaderStages{
        vtxStageInfo,
        fragStageInfo,
    };

    auto dynamicState = vk::PipelineDynamicStateCreateInfo{}
        .setDynamicStates(info.enabled_dynamic_states);

    auto vertexInputState = vk::PipelineVertexInputStateCreateInfo{}
        .setVertexBindingDescriptions(VertexTraits<VertexType>::binding_desc)
        .setVertexAttributeDescriptions(VertexTraits<VertexType>::attribute_desc);


    auto const inputAssemblyState=  vk::PipelineInputAssemblyStateCreateInfo{
        .topology = vk::PrimitiveTopology::eTriangleList,
    };

    auto viewportState = vk::PipelineViewportStateCreateInfo{
        // In this instance, since these are both dynamic state, 
        // we dont set them here, but rather during the draw call or smth
        .viewportCount = 1,
        .scissorCount = 1,
    };
    auto rasterizationState = vk::PipelineRasterizationStateCreateInfo{}
        .setDepthClampEnable        (vk::False)
        .setRasterizerDiscardEnable (vk::False)
        .setPolygonMode             (info.poly_mode)
        .setCullMode                (info.cull_mode) 
        .setFrontFace               (vk::FrontFace::eClockwise)
        .setDepthBiasEnable         (info.depth_bias)
        .setDepthBiasConstantFactor (info.depth_bias_constant)
        .setDepthBiasSlopeFactor    (info.depth_bias_slope)
        .setLineWidth               (1.0f)
    ;

    auto multisamplingState = vk::PipelineMultisampleStateCreateInfo{}
        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setSampleShadingEnable(vk::False)
    ;

    auto colorBlendAttachment = info.blend;

    auto colorBlendState = vk::PipelineColorBlendStateCreateInfo{
        .logicOpEnable = false,
        .logicOp = vk::LogicOp::eCopy,
    }.setAttachments(colorBlendAttachment);

    auto pipeline_layout = vk::raii::PipelineLayout{
        info.device,
        vk::PipelineLayoutCreateInfo{}
            .setPushConstantRangeCount(0)
            .setSetLayouts(*info.m_vkDescriptorSetLayout)
    };

    auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo{}
        .setDepthTestEnable(info.depth_test)
        .setDepthWriteEnable(info.depth_write)
        .setDepthCompareOp(info.depth_compar)
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
            .setLayout(pipeline_layout)
            .setPDepthStencilState(&depthStencilState)
            .setRenderPass(nullptr)
        ,
        vk::PipelineRenderingCreateInfo{}
            .setColorAttachmentCount(1)
            .setPColorAttachmentFormats( &info.color_image_format)
            .setDepthAttachmentFormat(info.depth_image_format )
    };
    return ShaderPipelineContext{
        vk::raii::Pipeline(
            info.device,
            nullptr,
            pipelineCreateInfoChain.template get<vk::GraphicsPipelineCreateInfo>()
        ),
        std::move(pipeline_layout)
    };
}
void Renderer::init_line_pipeline(vk::raii::ShaderModule const& shader_module) {
    m_line_pipeline = make_shader_pipeline<Vertex>(
        ShaderPipelineCreateInfo {
            .device                  = m_vkDevice,
            .shader_module           = shader_module,
            .vertex_fn_name          = "vertWireframe",
            .frag_fn_name            = "fragWireframe",
            .enabled_dynamic_states  = vk_enabledDynamicState,
            .m_vkDescriptorSetLayout = m_vkDescriptorSetLayout,
            .poly_mode               = vk::PolygonMode::eLine,
            .cull_mode               = vk::CullModeFlagBits::eNone,
            .color_image_format      = m_swapchain.imageFormat,
            .depth_image_format      = DepthAttachment::select_depth_format(),

            .depth_write             = false,
            .depth_test              = true,
            // Normally, this would be lessOrEqual, but we have a flipped Z.
            .depth_compar            = vk::CompareOp::eGreaterOrEqual,
            .blend                   = detail::helpers::no_blend(),
            .depth_bias              = false,
        }
    );

}

void Renderer::init_fill_pipeline(vk::raii::ShaderModule const& shader_module) {
    m_fill_pipeline = make_shader_pipeline<Vertex>(
        ShaderPipelineCreateInfo {
            .device                  = m_vkDevice,
            .shader_module           = shader_module,
            .vertex_fn_name          = "vertMain",
            .frag_fn_name            = "fragMain",
            .enabled_dynamic_states  = vk_enabledDynamicState,
            .m_vkDescriptorSetLayout = m_vkDescriptorSetLayout,
            .poly_mode               = vk::PolygonMode::eFill,
            .cull_mode               = vk::CullModeFlagBits::eNone,
            .color_image_format      = m_swapchain.imageFormat,
            .depth_image_format      = DepthAttachment::select_depth_format(),

            .depth_write             = true,
            .depth_test              = true,
            // Normally, this would be less, but we have a flipped Z.
            .depth_compar            = vk::CompareOp::eGreater,
            .blend                   = detail::helpers::no_blend(),
        }
    );
}


void Renderer::init_sync_structures() {
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


void Renderer::init_texture() {
    auto img_raw_data = ImageData::load_from_filename("./textures/tim_cheese.png");
    auto cmdBuf = begin_single_use_cmd();
    set_vk_dbg_name(m_vkDevice,cmdBuf, "Texture image transition layout buffer");
    m_texture.upload_image(m_allocator, cmdBuf, img_raw_data);
    m_texture.init_view(img_raw_data.vk_format, m_vkDevice);
    m_texture.init_sampler(m_vkDevice, m_vkPhysicalDevice);
}

