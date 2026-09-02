#pragma once

#include "vk_types.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_core.h>
namespace vk_util {
inline void transition_image_layout(
    vk::raii::CommandBuffer const& cmd, 
    vk::Image img, 
    vk::ImageLayout old_layout, 
    vk::ImageLayout new_layout,
    vk::AccessFlags2 src_access_mask,
    vk::AccessFlags2 dst_access_mask,
    vk::PipelineStageFlags2 src_stage_mask,
    vk::PipelineStageFlags2 dst_stage_mask,
    vk::ImageAspectFlags aspectMask
) {


    auto imgBarrier = vk::ImageMemoryBarrier2KHR{
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,

        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,

        .oldLayout = old_layout,
        .newLayout = new_layout,

        .srcQueueFamilyIndex = vk::QueueFamilyIgnored, 
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored, 

        .image = img,
        .subresourceRange = vk::ImageSubresourceRange{
            .aspectMask = aspectMask,
            .baseMipLevel = 0,
            .levelCount = vk::RemainingMipLevels,
            .baseArrayLayer = 0,
            .layerCount = vk::RemainingArrayLayers,
        },
    };

    cmd.pipelineBarrier2(
        vk::DependencyInfo{
            .dependencyFlags = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &imgBarrier
        }
    );

}

} // namespace vk_util
