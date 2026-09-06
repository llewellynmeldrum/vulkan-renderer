#pragma once 
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "vk_concepts.hpp"
#include "vk_types.hpp"

namespace detail::debug{
VKAPI_ATTR vk::Bool32 VKAPI_CALL 
vk_debug_callback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT types,
    vk::DebugUtilsMessengerCallbackDataEXT const* data,
    void* _
) ;

template<typename T>
static consteval auto get_vk_object_type(){
    if constexpr (isVkRaiiType<T>){
        return T::CppType::objectType;
    }else if constexpr (IsCppHandle<T>){
        return T::objectType;
    }else {
        return vk::ObjectType::eUnknown;
    }
}
}

// required to extend the lifetime of the object names we use for debugging (i think?)
inline std::vector<std::string> object_name_registry;
template <typename T>
inline void set_vk_dbg_name(vk::raii::Device const& device, T const& object, std::string name_in) {
    vk::DebugUtilsObjectNameInfoEXT nameInfo;
    auto name = object_name_registry.emplace_back(std::move(name_in));
    
    nameInfo.objectType   = detail::debug::get_vk_object_type<T>();
    nameInfo.objectHandle = reinterpret_cast<uint64_t>(detail::debug::get_c_handle(object));
    nameInfo.pObjectName  = name.c_str(); // should live for as long as the engine does

    device.setDebugUtilsObjectNameEXT(nameInfo);
} 
