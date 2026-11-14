#include <cpptrace/basic.hpp>
#include <sstream>
#include <string>

#include <cpptrace/formatting.hpp>
#include <libassert/platform.hpp>

#include "vk_debug.hpp"
#include "debug.hpp"
#include "style.hpp"
#include "primitive_types.hpp"

namespace detail::debug{
static std::string format_vk_error(vk::DebugUtilsMessengerCallbackDataEXT const* d){
    auto res = std::string{};
//    const char*                                  pMessageIdName;
//    int32_t                                      messageIdNumber;
//    const char*                                  pMessage;
//    uint32_t                                     queueLabelCount;
//    const VkDebugUtilsLabelEXT*                  pQueueLabels;
//    uint32_t                                     cmdBufLabelCount;
//    const VkDebugUtilsLabelEXT*                  pCmdBufLabels;
//    uint32_t                                     objectCount;
//    const VkDebugUtilsObjectNameInfoEXT*         pObjects;
    //
    
    auto header = std::format("VULKAN ERROR [{}]: \n", d->pMessageIdName);
    auto body = std::format("{}\n",d->pMessage);

    auto q_header  =std::format("queue entries (n={}):\n",d->queueLabelCount);
    auto q = std::string{};
    for (u32 i =0 ; i<d->queueLabelCount; i++){
        q += std::format("\t[{}]-> {}, \n",i,d->pQueueLabels[i].pLabelName);
    }
    auto cb_header =std::format("Command buffer entries(n={}):\n",d->cmdBufLabelCount);
    auto cb = std::string{};
    for (u32 i =0 ; i<d->cmdBufLabelCount; i++){
        cb += std::format("\t[{}]-> {}, \n",i,d->pCmdBufLabels[i].pLabelName);
    }
    auto objs_header =std::format("Related objects (n={}):\n",d->objectCount);
    auto objs = std::string{};
    for (u32 i =0 ; i<d->objectCount; i++){
        objs += std::format("\t[{}]->",i);
        objs += std::format("vk::{}", vk::to_string(d->pObjects[i].objectType));
        if (d->pObjects[i].pObjectName){
            objs += std::format("'{}'", d->pObjects[i].pObjectName);
        }else{
            objs += std::format("(unnamed)");
        }
        objs+="\n";
    }


    return std::format(
        "{}{}{}{}{}{}{}{}",
        style::with_fg(style::fg_br_red(),header),
        style::with_fg(style::fg_red(),body),
        style::with_fg(style::fg_br_red(),q_header),
        style::with_fg(style::fg_br_red(),q),
        style::with_fg(style::fg_br_red(),cb_header),
        style::with_fg(style::fg_br_red(),cb),
        style::with_fg(style::fg_br_red(),objs_header),
        style::with_fg(style::fg_br_red(),objs)
    );
}
VKAPI_ATTR vk::Bool32 VKAPI_CALL 
vk_debug_callback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT types,
    vk::DebugUtilsMessengerCallbackDataEXT const* data,
    [[maybe_unused]] void* user_ptr
) {
    if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) {
        std::cerr << std::format("VULKAN ERROR! {} ", format_vk_error(data));
        dbg::exit();
    }
    return vk::False;
}
} // namespace detail::debug
