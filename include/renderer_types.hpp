#pragma once 
#include "shared.hpp"
#include "Texture2D.hpp"
#include "input_keycodes.hpp"
#include "vk_managed_buffers.hpp"
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
struct ShaderPipelineContext{
    vk::raii::Pipeline vk_pipeline{nullptr};
    vk::raii::PipelineLayout layout{nullptr};
    inline void clear(){
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

struct CommandContext{
    vk::raii::CommandBuffer const& cmdBuf;
    u32 imageIndex;
    ShaderPipelineContext const& pipeline;
};


struct ShaderPipelineCreateInfo{
    vk::raii::Device const& device;
    vk::raii::ShaderModule const& shader_module;
    std::string vertex_fn_name;
    std::string frag_fn_name;
    std::span<const vk::DynamicState> enabled_dynamic_states;
    vk::raii::DescriptorSetLayout const& m_vkDescriptorSetLayout;

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


};
