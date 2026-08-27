#pragma once 
#include <vulkan/vulkan.hpp>
VKAPI_ATTR vk::Bool32 VKAPI_CALL vk_debug_callback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT types,
    vk::DebugUtilsMessengerCallbackDataEXT const* data,
    void* _
) ;
