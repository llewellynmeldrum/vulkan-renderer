#pragma once 
#include "vk_buffers_helpers.hpp"
#include "vk_types.hpp"

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
