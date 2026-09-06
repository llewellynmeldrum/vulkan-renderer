#include "vk_managed_buffers.hpp"
#include <cstdlib>
#include <optional>
#include <print>

#include <format>
AllocatedBuffer::AllocatedBuffer(
    VmaAllocator const a_allocator,
    vk::BufferCreateInfo bufInfo,
    VmaAllocationCreateInfo allocCreateInfo,
     std::string_view opt_name
) 
    :allocator(a_allocator)
{
//    LOG_DBG("====\nCreating allocation for buffer '{}'",opt_name);

    std::string flags = vk::to_string(bufInfo.flags                );
    std::string size = std::format("{}", bufInfo.size                 );
    std::string usage = vk::to_string(bufInfo.usage                );
    std::string sharingMode = vk::to_string(bufInfo.sharingMode          );
    std::string queueFamilyIndexCount = std::format("{}", bufInfo.queueFamilyIndexCount);
    std::string pQueueFamilyIndices = std::format("{}", static_cast<const void*>(bufInfo.pQueueFamilyIndices ));
//    std::println(
//        "\t.flags                 ={}\n"
//        "\t.size                  ={}\n"
//        "\t.usage                 ={}\n"
//        "\t.sharingMode           ={}\n"
//        "\t.queueFamilyIndexCount ={}\n"
//        "\t.pQueueFamilyIndices   ={}\n",
//        flags ,
//        size ,
//        usage ,
//        sharingMode ,
//        queueFamilyIndexCount ,
//        pQueueFamilyIndices 
//    );
    ASSERT(bufInfo.size > 0, "Vulkan does not allow buffers of size 0.");
    auto rawBuffer = VkBuffer{};
    auto allocOutInfo = VmaAllocationInfo{};
    auto res = vmaCreateBuffer(
        allocator,
        reinterpret_cast<VkBufferCreateInfo const*>(&bufInfo),
        &allocCreateInfo,
        &rawBuffer,
        &allocation,
        &allocOutInfo
    );
    if (res != VkResult::VK_SUCCESS){
        LOG_ERROR("VMA Failed to allocate buffer: {}",vk::to_string(vk::Result{res}));
        LOG_EXIT(EXIT_FAILURE);
    }

    static constexpr auto transfer_usage = vk::BufferUsageFlagBits::eTransferSrc;
    static constexpr auto vma_transfer_flags = 
        VMA_ALLOCATION_CREATE_MAPPED_BIT 
        | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    if ((bufInfo.usage &  transfer_usage) == transfer_usage){
        ASSERT((allocCreateInfo.flags & vma_transfer_flags) == vma_transfer_flags);
        mapped_ptr = reinterpret_cast<std::byte*>(allocOutInfo.pMappedData);
    }
    buffer = rawBuffer;
}

void AllocatedBuffer::clear(){
    if (allocator != nullptr && (buffer || allocation)){
        vmaDestroyBuffer(allocator, buffer, allocation);
    }
    buffer     = nullptr;
    allocation = nullptr;
    mapped_ptr = nullptr;
}


AllocatedImage::AllocatedImage(
    VmaAllocator const a_allocator,
    vk::ImageCreateInfo imgInfo,
    VmaAllocationCreateInfo allocCreateInfo,
     std::string_view opt_name
) 
    :allocator(a_allocator)
{
//    LOG_DBG("====\nCreating allocation for buffer '{}'",opt_name);

    auto rawImage = VkImage{};
    auto allocOutInfo = VmaAllocationInfo{};
    auto res = vmaCreateImage(
        allocator,
        reinterpret_cast<VkImageCreateInfo const*>(&imgInfo),
        &allocCreateInfo,
        &rawImage,
        &allocation,
        &allocOutInfo
    );
    if (res != VkResult::VK_SUCCESS){
        LOG_ERROR("VMA Failed to allocate buffer: {}",vk::to_string(vk::Result{res}));
        LOG_EXIT(EXIT_FAILURE);
    }

    static constexpr auto transfer_usage = vk::ImageUsageFlagBits::eTransferSrc;
    static constexpr auto vma_transfer_flags = 
        VMA_ALLOCATION_CREATE_MAPPED_BIT 
        | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    if ((imgInfo.usage &  transfer_usage) == transfer_usage){
        ASSERT((allocCreateInfo.flags & vma_transfer_flags) == vma_transfer_flags);
        mapped_ptr = reinterpret_cast<std::byte*>(allocOutInfo.pMappedData);
    }
    image = rawImage;
}

void AllocatedImage::clear(){
    if (allocator != nullptr && (image || allocation)){
        vmaDestroyImage(allocator, image, allocation);
    }
    image     = nullptr;
    allocation = nullptr;
    mapped_ptr = nullptr;
}


