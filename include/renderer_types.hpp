#pragma once 
#include "shared.hpp"
#include "Texture2D.hpp"
#include "input_keycodes.hpp"
#include "vk_managed_buffers.hpp"
#include "push_constants_concepts.hpp"
#include "vk_types.hpp"
#include "vk_frame_data.hpp"
#include "vk_swapchain.hpp"
#include "vk_debug.hpp"
#include "cpu_mesh.hpp"
#include "gpu_mesh.hpp"
#include "heightmap.hpp"
#include "depth_image.hpp"
#include "shared_transformations.hpp"
#include "vk_swapchain.hpp"
#include "vk_types.hpp"
#include "renderer_buffer_helpers.hpp"
#include "camera.hpp"

// The pipeline stores:
// - pipeline object 
// - pipeline layout
// - dynamic states
// - The type of push constant (if any)
template<typename tPushConstantType>
    requires PushConstants::is_valid<tPushConstantType>
struct Pipeline{
    using PushConstantType = tPushConstantType;

    vk::raii::Pipeline vk_pipeline{nullptr};
    vk::raii::PipelineLayout layout{nullptr};

    struct DynamicStates{
        vk::PolygonMode poly_mode;
    }dynamic;

    std::optional<vk::PushConstantRange> pc_info = std::nullopt;

    auto push(vk::raii::CommandBuffer const& cmd, PushConstantType const& pc_type) 
    -> void{
        ASSERT(pc_info != std::nullopt, "Error: pipeline not setup with push constants tried to push one!");
        auto const* ptr = reinterpret_cast<void const*>(&pc_type);
        cmd.pushConstants(layout,pc_info->stageFlags,pc_info->offset,pc_info->size, ptr);
    }

    auto prepare_pass(
        vk::raii::CommandBuffer const& cmdBuf,
        vk::raii::DescriptorSet const& descriptorSet
    ) -> void{
        cmdBuf.bindPipeline(
            vk::PipelineBindPoint::eGraphics,
            *vk_pipeline
        );
        cmdBuf.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            layout,
            0, 
            *descriptorSet,
            nullptr
        );
        cmdBuf.setPolygonModeEXT(dynamic.poly_mode);
    }
    auto clear() -> void{
        layout.clear();
        vk_pipeline.clear();
    };
};

struct RenderAttachmentContext{
    vk::RenderingAttachmentInfo colorAttachment;
    vk::RenderingAttachmentInfo depthAttachment;
    auto get_info(Swapchain const& swapchain) const{
        return vk::RenderingInfo{}
            .setRenderArea({.offset={},.extent=swapchain.extent})
            .setLayerCount(1)
            .setColorAttachments(colorAttachment)
            .setPDepthAttachment(&depthAttachment)
        ;
    }

};


struct PipelineCreateInfo{
    static constexpr inline bool extra_check_on_pipeline = true;

    std::string_view pipeline_name = "n/a";
    std::string_view shader_module_name = "n/a";
    vk::raii::ShaderModule const& shader_module;
    vk::raii::Device const& device;
    std::string vertex_fn_name;
    std::string frag_fn_name;
    std::span<const vk::DynamicState> enabled_dynamic_states;
    vk::raii::DescriptorSetLayout const& m_vkDescriptorSetLayout; 
    // TODO: make this optional type or something
    // for pipelines which dont use a UBO at all 

    vk::PolygonMode poly_mode;
    vk::CullModeFlags cull_mode;

    vk::Format color_image_format;
    vk::Format depth_image_format;

    bool depth_write = true;
    bool depth_test = true;
    vk::CompareOp depth_compar = vk::CompareOp::eLess;
    vk::PipelineColorBlendAttachmentState blend = detail::helpers::no_blend();

    bool depth_bias = false;
    f32 depth_bias_constant = 0.0f;
    f32 depth_bias_slope = 0.0f;

    vk::PrimitiveTopology primitive_topology = vk::PrimitiveTopology::eTriangleList;


};
