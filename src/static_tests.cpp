#include "common_concepts.hpp"
#include "renderer_concepts.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

static_assert(vkhpp_handle<vk::Instance>); static_assert(!vkhpp_raii<vk::Instance>);

static_assert(!vkhpp_handle<vk::raii::Instance>); static_assert(vkhpp_raii<vk::raii::Instance>);
