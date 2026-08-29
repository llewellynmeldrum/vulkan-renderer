#pragma once 
#include <mutex>
#include <type_traits>
#include <vulkan/vulkan_raii.hpp>

struct FrameData {
    vk::raii::CommandPool commandPool{nullptr};
    vk::raii::CommandBuffer commandBuffer{nullptr};
    vk::raii::Fence fence{nullptr};
    vk::raii::Semaphore presentCompleteSemaphore{nullptr};


    vk::raii::Buffer uniformBuffer{nullptr};
    vk::raii::DeviceMemory uniformBufferMemory{nullptr};
    void* uniformBufferMappedMemory{nullptr};


};

static_assert(std::is_aggregate_v<FrameData>);
static_assert(std::is_default_constructible_v<FrameData>);
static_assert(std::move_constructible<FrameData>);



