#pragma once 
#include  "vk_types.hpp"
inline void transition_img_layout(
    vk::raii::CommandBuffer const& cmd
    ,vk::Image const& img
    ,vk::ImageLayout old_layout
    ,vk::ImageLayout new_layout
    ,vk::ImageAspectFlags image_aspect_flags
){
    auto barrier = vk::ImageMemoryBarrier{}
        .setOldLayout(old_layout)
        .setNewLayout(new_layout)
        .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setImage(img)
        .setSubresourceRange(
            vk::ImageSubresourceRange{}
                .setAspectMask(image_aspect_flags)
                .setLevelCount(1)
                .setLayerCount(1)
        )
    ;
    auto src_stage = vk::PipelineStageFlags{};
    auto dst_stage = vk::PipelineStageFlags{};

    if (old_layout == vk::ImageLayout::eUndefined 
        && 
        new_layout == vk::ImageLayout::eTransferDstOptimal
    ){
        barrier.setSrcAccessMask({});
        barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

        src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        dst_stage = vk::PipelineStageFlagBits::eTransfer;
    }else if (old_layout == vk::ImageLayout::eTransferDstOptimal
              && 
              new_layout == vk::ImageLayout::eShaderReadOnlyOptimal
    ){
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        src_stage = vk::PipelineStageFlagBits::eTransfer;
        dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
    }else {
        LOG_FATAL("Unsupported image layout transition ({})->({})",
                  vk::to_string(old_layout),vk::to_string(new_layout));
    }

    cmd.pipelineBarrier(src_stage, dst_stage, {}, {}, {}, barrier);
}
inline void copy_buffer_to_image(
    vk::raii::CommandBuffer const& cmd, 
    vk::Buffer const& src_buffer, 
    vk::Image & dst_image,
    vk::Extent2D img_extent
){
    cmd.copyBufferToImage(
        src_buffer, 
        dst_image,
        vk::ImageLayout::eTransferDstOptimal,
        vk::BufferImageCopy{}
            .setBufferOffset(0)
            .setBufferRowLength(0)
            .setBufferImageHeight(0)
            .setImageSubresource(
                vk::ImageSubresourceLayers{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setMipLevel(0)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1)
            )
            .setImageOffset({0,0,0})
            .setImageExtent({img_extent.width,img_extent.height,1})

    );
}
