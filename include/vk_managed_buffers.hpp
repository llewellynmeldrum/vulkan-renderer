#pragma once 

#include "glm/ext/matrix_float4x4.hpp"
#include "vk_types.hpp"
#include <vulkan/vulkan_raii.hpp>

struct UniformBufferObject{
    alignas(16) glm::mat4x4 model;
    alignas(16) glm::mat4x4 view;
    alignas(16) glm::mat4x4 proj;
};

struct AllocatedImage{
    AllocatedImage(nullptr_t) {}
    AllocatedImage(
        VmaAllocator const a_allocator,
        vk::ImageCreateInfo bufInfo,
        VmaAllocationCreateInfo allocInfo,
        std::string_view opt_name=""
    );
    AllocatedImage(AllocatedImage &&o) noexcept
        : allocator(std::exchange(o.allocator, nullptr))
        , image(std::exchange(o.image, nullptr))
        , allocation(std::exchange(o.allocation, nullptr))
        , mapped_ptr(std::exchange(o.mapped_ptr, nullptr))
    {}
    AllocatedImage(AllocatedImage const&) = delete;
    AllocatedImage &operator=(AllocatedImage const &) = delete;

    AllocatedImage &operator=(AllocatedImage &&o) noexcept {
        if (this != &o) {
            clear();
            allocator = std::exchange(o.allocator, nullptr);
            image = std::exchange(o.image, nullptr);
            allocation = std::exchange(o.allocation, nullptr);
            mapped_ptr = std::exchange(o.mapped_ptr, nullptr);
        }
        return *this;
    }
    ~AllocatedImage(){clear();};
    void clear();

    VmaAllocator allocator{};
    vk::Image image;
    VmaAllocation allocation{};
    std::byte* mapped_ptr{nullptr};
};

struct AllocatedBuffer{
    AllocatedBuffer(nullptr_t) {}
    AllocatedBuffer(VmaAllocator const a_allocator, vk::BufferCreateInfo bufInfo,
                  VmaAllocationCreateInfo allocInfo, std::string_view opt_name="");
    AllocatedBuffer(AllocatedBuffer &&o) noexcept
    : allocator(std::exchange(o.allocator, nullptr))
    , buffer(std::exchange(o.buffer, nullptr))
    , allocation(std::exchange(o.allocation, nullptr))
    , mapped_ptr(std::exchange(o.mapped_ptr, nullptr))
    {}
    AllocatedBuffer(AllocatedBuffer const&) = delete;
    AllocatedBuffer &operator=(AllocatedBuffer const &) = delete;

    AllocatedBuffer &operator=(AllocatedBuffer &&o) noexcept {
        if (this != &o) {
            clear();
            allocator = std::exchange(o.allocator, nullptr);
            buffer = std::exchange(o.buffer, nullptr);
            allocation = std::exchange(o.allocation, nullptr);
            mapped_ptr = std::exchange(o.mapped_ptr, nullptr);
        }
        return *this;
    }
    ~AllocatedBuffer(){clear();};
    void clear();

    VmaAllocator allocator{};
    vk::Buffer buffer{};
    VmaAllocation allocation{};
    std::byte* mapped_ptr{nullptr};
};

