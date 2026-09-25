#pragma once 
#include "vk_debug.hpp"
#include "vk_image_data.hpp"
#include "vk_types.hpp"
#include "vk_managed_buffers.hpp"
#include "vk_image_helpers.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

// 
struct Texture2D{
    AllocatedImage textureImage{nullptr};
    vk::raii::Sampler sampler{nullptr};
    vk::raii::ImageView image_view{nullptr};

    vk::ImageLayout layout{};
    vk::DescriptorImageInfo descriptor_image_info{nullptr};
    vk::Format image_format{};

    static constexpr auto descriptor_type = vk::DescriptorType::eCombinedImageSampler;
    auto clear() -> void {
        textureImage.clear();
        sampler.clear();
        image_view.clear();
    }


    auto get_write_descriptor_set(
        vk::DescriptorSet const& descriptor_set
    ) const noexcept{
        return vk::WriteDescriptorSet{}
                .setDstSet(descriptor_set)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setImageInfo(descriptor_image_info)
        ;
    }

    // NOTE: 
    // The following 'make_*' style functions are all static, in order to force us to know their requirements/
    // dependencies, making it less likely to call them out of order.
    // For example, you cant accidentally call make_view() before uploading the texture image, since the first arg of
    // make_view() IS the texture image.

    [[nodiscard]]
    static constexpr auto make_descriptor_image_info(
        vk::raii::Sampler const& sampler,
        vk::raii::ImageView const& image_view,
        vk::ImageLayout layout
    ) noexcept -> vk::DescriptorImageInfo{
        return vk::DescriptorImageInfo{}
            .setSampler(sampler)
            .setImageView(image_view)
            .setImageLayout(layout)
         ;
    }
    [[nodiscard]]
    static constexpr auto make_sampler(
        vk::raii::Device const& device,
        vk::raii::PhysicalDevice const& physical_device
    ) noexcept -> vk::raii::Sampler{
        return vk::raii::Sampler(
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
    [[nodiscard]]
    static constexpr auto make_view(
        AllocatedImage const& texture_image,
        vk::Format format,
        vk::raii::Device const& device
    )->vk::raii::ImageView{
        return vk::raii::ImageView(
            device,
            vk::ImageViewCreateInfo{}
                .setImage(texture_image.image)
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
};
