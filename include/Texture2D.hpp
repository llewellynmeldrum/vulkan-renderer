#pragma once 
#include "vk_image_data.hpp"
#include "vk_types.hpp"
#include "vk_managed_buffers.hpp"
#include "vk_image_helpers.hpp"
#include <vulkan/vulkan_raii.hpp>

struct Texture2D{
    AllocatedImage textureImage;
    vk::raii::Sampler sampler{nullptr};
    vk::raii::ImageView image_view{nullptr};
    void init_sampler(
        vk::raii::Device const& device,
        vk::raii::PhysicalDevice const& physical_device
    ){
        sampler = vk::raii::Sampler(
            device,
            vk::SamplerCreateInfo{}
                .setMagFilter(vk::Filter::eNearest)
                .setMinFilter(vk::Filter::eNearest)
                .setMipmapMode(vk::SamplerMipmapMode::eNearest)
                .setMipLodBias(0.0f)
                .setMinLod(0.0f)
                .setMaxLod(0.0f)
                .setAddressModeU(vk::SamplerAddressMode::eRepeat)
                .setAddressModeV(vk::SamplerAddressMode::eRepeat)
                .setAddressModeW(vk::SamplerAddressMode::eRepeat)
                .setAnisotropyEnable(vk::True)
                .setMaxAnisotropy(physical_device.getProperties().limits.maxSamplerAnisotropy)
                .setCompareEnable(vk::False)
                .setCompareOp(vk::CompareOp::eAlways)
                .setBorderColor(vk::BorderColor::eIntOpaqueWhite)
                .setUnnormalizedCoordinates(vk::False)
                
        );
    }
    void init_view(
        vk::Format format,
        vk::raii::Device const& device
    ){
        image_view = vk::raii::ImageView(
            device,
            vk::ImageViewCreateInfo{}
                .setImage(textureImage.image)
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(format)
                .setSubresourceRange(
                    vk::ImageSubresourceRange{}
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setBaseMipLevel(0)
                        .setBaseArrayLayer(0)
                        .setLayerCount(1)
                        .setLevelCount(1)
                )
        );
    }
    void upload_image(
        VmaAllocator const m_allocator,
        vk::raii::CommandBuffer const& cmd,
        ImageData img_raw_data
    ){
        ASSERT(img_raw_data.size_bytes() > 0, "Vulkan does not support buffers of 0 bytes without extensions");
        auto stagingBuffer = AllocatedBuffer(m_allocator,
            vk::BufferCreateInfo{}
                .setSize(img_raw_data.size_bytes())
                .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
                .setSharingMode(vk::SharingMode::eExclusive)
            ,
            VmaAllocationCreateInfo{
                .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO
            },
            "Texture image Staging buf"
        );
        LOG_DBG("Texture2D.uploadImage :  Allocated staging buffer");
        ASSERT(stagingBuffer.mapped_ptr);
        // 2. Copy the data into the staging buffer

        auto image_cpu_to_staging_res = VkResult{};
        image_cpu_to_staging_res = vmaCopyMemoryToAllocation(
            m_allocator,
            img_raw_data.span.data(),
            stagingBuffer.allocation,
            0,
            vk::DeviceSize{img_raw_data.size_bytes()}
        );
        if (image_cpu_to_staging_res != VkResult::VK_SUCCESS){
            LOG_FATAL("Failed to copy cpu image data into staging buffer.");
        }
        img_raw_data.free_buffer();
        LOG_DBG("Texture2D.uploadImage :  Uploaded image data to staging buffer");

        // 2. create the texture image
        textureImage = AllocatedImage(m_allocator,
            vk::ImageCreateInfo{}
                .setImageType(vk::ImageType::e2D)
                .setFormat(img_raw_data.vk_format)
                .setExtent({static_cast<u32>(img_raw_data.px_w),static_cast<u32>(img_raw_data.px_h),1})
                .setMipLevels(1)
                .setArrayLayers(1)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setTiling(vk::ImageTiling::eOptimal)
                .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
                .setSharingMode(vk::SharingMode::eExclusive)
            ,
            VmaAllocationCreateInfo{
                .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
            },
            "Texture image buffer"
        );
        LOG_DBG("Texture2D.uploadImage :  Created allocated image");

        transition_img_layout(
            cmd,
            textureImage.image, 
            vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
            vk::ImageAspectFlagBits::eColor
        );
        copy_buffer_to_image(cmd,stagingBuffer.buffer, textureImage.image, img_raw_data.get_extent2d());
        transition_img_layout(
            cmd,
            textureImage.image, 
            vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::ImageAspectFlagBits::eColor
        );
        LOG_DBG("Texture2D.uploadImage :  Copied staging buffer data to allocated image.");

    }
};
