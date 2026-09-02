#pragma once 

#include "glm/ext/matrix_float4x4.hpp"
#include "vk_types.hpp"

struct UniformBufferObject{
    alignas(16) glm::mat4x4 model;
    alignas(16) glm::mat4x4 view;
    alignas(16) glm::mat4x4 proj;
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
    AllocatedBuffer(const AllocatedBuffer &) = delete;
    AllocatedBuffer &operator=(const AllocatedBuffer &) = delete;
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

