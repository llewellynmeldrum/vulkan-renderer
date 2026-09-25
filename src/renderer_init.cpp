#include <thread>
#include <span>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#include "SDL3/SDL_vulkan.h"
#include "heightmap.hpp"
#include "push_constants_traits.hpp"
#include "push_constants.hpp"
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

    try{
        init_vulkan();
    }catch(vk::SystemError const& e){
        LOG_FATAL("Failed to initialize vulkan: {}", e.what());
    }
}


void Renderer::init_vulkan() {
    //clang-format off
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
        m_rend2d.init();
        m_rend2d.font_atlas = make_font_atlas(
            "./fonts/basis33/basis33.ttf"
        );
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
            m_rend2d.font_atlas.atlas_texture,
            k_syncFrameCount
        );
    }


    {
        auto main_shaders_module = detail::helpers::make_shader_module(
            m_vkDevice, 
            "./shaders/bin/main_shaders.spv",
            "main_shaders"
        );

        init_fill_pipeline(main_shaders_module);
        init_line_pipeline(main_shaders_module);

        auto shaders2d_module = detail::helpers::make_shader_module(
            m_vkDevice, 
            "./shaders/bin/shaders2d.spv",
            "shaders2d"
        );
        init_2d_pipeline(shaders2d_module);
    }

    {
        init_sync_structures();
    }
}




//void init_vma(
void Renderer::cleanup_vma()const noexcept{
    vmaDestroyAllocator(m_allocator);
}






template<typename VertexType, typename PushConstantType=PushConstants::None>
auto make_shader_pipeline(PipelineCreateInfo info)
-> Pipeline<PushConstantType>{

    if (PipelineCreateInfo::extra_check_on_pipeline){ detail::perform_extra_pipeline_check(info); }
    
    auto vertexInputState = VertexInputState<VertexType>::get();
    auto shaderStages = detail::make_shader_stages(info);
    auto rasterizationState = detail::get_raster_state(info);


    auto dynamicState = vk::PipelineDynamicStateCreateInfo{}
        .setDynamicStates(info.enabled_dynamic_states)
    ;
    auto const inputAssemblyState =  vk::PipelineInputAssemblyStateCreateInfo{}
        .setTopology(info.primitive_topology)
    ;

    // In this instance, since these are both dynamic state, 
    // we dont set them here, but rather during the draw call or smth
    auto viewportState = vk::PipelineViewportStateCreateInfo{
        .viewportCount = 1,
        .scissorCount = 1,
    };


    auto multisamplingState = vk::PipelineMultisampleStateCreateInfo{}
        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setSampleShadingEnable(vk::False)
    ;


    auto colorBlendState = vk::PipelineColorBlendStateCreateInfo{}
        .setLogicOpEnable(false)
        .setLogicOp(vk::LogicOp::eCopy)
        .setAttachments(info.blend)
    ;

    auto push_constant_ranges = PushConstants::Traits<PushConstantType>::ranges;
    auto pipeline_layout = vk::raii::PipelineLayout{
        info.device,
        vk::PipelineLayoutCreateInfo{}
            .setPushConstantRanges(push_constant_ranges)
            .setSetLayouts(*info.descriptor_set_layout)
    };

    auto depthStencilState = detail::make_depth_stencil_state(info);

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
            .setDepthAttachmentFormat(info.depth_image_format)
    };
    static_assert(push_constant_ranges.size() ==0 || push_constant_ranges.size() ==1, "Need to modify ctor below if this changes");
    auto pipeline = Pipeline<PushConstantType>{
        .vk_pipeline = vk::raii::Pipeline(
            info.device,
            nullptr,
            pipelineCreateInfoChain.template get<vk::GraphicsPipelineCreateInfo>()
        ),
        .layout = std::move(pipeline_layout),
        .dynamic = {
            .poly_mode = info.poly_mode
        },
    };
    if constexpr (!std::same_as<PushConstantType, PushConstants::None>){
        pipeline.pc_info = push_constant_ranges.at(0);
    }
    return pipeline;
}
void Renderer::init_line_pipeline(detail::helpers::ShaderModuleWrapper const& shader) {
    m_line_pipeline = make_shader_pipeline<Vertex3D, PushConstants::ModelMatrix>(
        PipelineCreateInfo {
            .pipeline_name           = "main_line",
            .shader_module_name      = shader.module_name,
            .shader_module           = shader.module,
            .device                  = m_vkDevice,
            .vertex_fn_name          = "vertWireframe",
            .frag_fn_name            = "fragWireframe",
            .enabled_dynamic_states  = vk_enabledDynamicState,
            .descriptor_set_layout = m_vkDescriptorSetLayout,
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

auto Renderer::init_2d_pipeline(detail::helpers::ShaderModuleWrapper const& shader) 
-> void{
    m_2d_pipeline = make_shader_pipeline<
        Vertex2D, // vertex type 
        PushConstants::Transform2D // push constant type
    > (
        PipelineCreateInfo {
            .pipeline_name           = "2d",
            .shader_module_name      = shader.module_name,
            .shader_module           = shader.module,
            .device                  = m_vkDevice,
            .vertex_fn_name          = "vertMain",
            .frag_fn_name            = "fragMain",
            .enabled_dynamic_states  = vk_enabledDynamicState,
            .descriptor_set_layout = m_vkDescriptorSetLayout,
            .poly_mode               = vk::PolygonMode::eFill,
            .cull_mode               = vk::CullModeFlagBits::eNone,
            .color_image_format      = m_swapchain.imageFormat,
            .depth_image_format      = DepthAttachment::select_depth_format(),

            .depth_write             = false,
            .depth_test              = true,
            // Normally, this would be lessOrEqual, but we have a flipped Z.
            .depth_compar            = vk::CompareOp::eGreaterOrEqual,
            .blend                   = detail::helpers::alpha_blend(),
            .depth_bias              = false,
        }
    );

}

void Renderer::init_fill_pipeline(detail::helpers::ShaderModuleWrapper const& shader) {
    m_fill_pipeline = make_shader_pipeline<Vertex3D, PushConstants::ModelMatrix>(
        PipelineCreateInfo{
            .pipeline_name           = "main_fill",
            .shader_module_name      = shader.module_name,
            .shader_module           = shader.module,
            .device                  = m_vkDevice,
            .vertex_fn_name          = "vertMain",
            .frag_fn_name            = "fragMain",
            .enabled_dynamic_states  = vk_enabledDynamicState,
            .descriptor_set_layout = m_vkDescriptorSetLayout,
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
    // all i need is to be able to supply texture2D's pctor with a function it can call
    m_texture = make_texture_2d( "./textures/tim_cheese.png");
}


