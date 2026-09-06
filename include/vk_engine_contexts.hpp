#pragma once 
#include "vk_swapchain.hpp"
#include "vk_types.hpp"
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

