#pragma once 
#include "vk_managed_buffers.hpp"
#include "vk_types.hpp"

struct DepthAttachment{
    AllocatedImage img{nullptr};
    vk::raii::ImageView img_view{nullptr};
    static DepthAttachment make(
        vk::raii::Device const& m_vkDevice,
        VmaAllocator const m_allocator,
        vk::Extent2D swap_extent
    ){
        auto res = DepthAttachment{};

        // 2. create the texture image
        res.img = AllocatedImage(m_allocator,
            vk::ImageCreateInfo{}
                .setImageType(vk::ImageType::e2D)
                .setFormat(select_depth_format())
                .setExtent({swap_extent.width,swap_extent.height,1})
                .setMipLevels(1)
                .setArrayLayers(1)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setTiling(vk::ImageTiling::eOptimal)
                .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
                .setSharingMode(vk::SharingMode::eExclusive)
            ,
            VmaAllocationCreateInfo{
                .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
            },
            "Depth image"
        );
        res.init_view(m_vkDevice);

        return res;

        
    }
    static inline vk::Format select_depth_format(){
        // TODO: if bothered, make a selector here, since some devices dont support it 
        return vk::Format::eD32Sfloat;
    }
private:
    void init_view(
        vk::raii::Device const& m_vkDevice
    ){
        img_view = vk::raii::ImageView(
            m_vkDevice,
            vk::ImageViewCreateInfo{}
                .setImage(img.image)
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(select_depth_format())
                .setSubresourceRange(
                    vk::ImageSubresourceRange{}
                        .setAspectMask(vk::ImageAspectFlagBits::eDepth)
                        .setBaseMipLevel(0)
                        .setBaseArrayLayer(0)
                        .setLayerCount(1)
                        .setLevelCount(1)
                )
        );
    }
};
