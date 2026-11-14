#include <imgui_impl_vulkan.h>
#include "Gui.hpp"
#include "platform.hpp"
#include "Engine_Gui.hpp"
#include "r3d.hpp"

auto init_gui(Gui& ui, Platform& platform, Renderer3D& r3d) -> void{
    using detail::debug::get_c_handle;
    auto const color_format = static_cast<VkFormat>(r3d.m_gpuContext.swapchain.imageFormat);
    auto const depth_format = static_cast<VkFormat>(DepthAttachment::select_depth_format());

    auto info = ImGui_ImplVulkan_InitInfo{
        .ApiVersion = r3d.k_apiVersion,
        .Instance = get_c_handle(r3d.m_gpuContext.instance),
        .PhysicalDevice = get_c_handle(r3d.m_gpuContext.physicalDevice),
        .Device = get_c_handle(r3d.m_gpuContext.device),
        .QueueFamily = r3d.m_gpuContext.queueFamily,
        .Queue = get_c_handle(r3d.m_gpuContext.queue),
        .DescriptorPoolSize = 64,
        .MinImageCount = r3d.k_syncFrameCount,
        .ImageCount = r3d.k_syncFrameCount,
        .PipelineInfoMain = ImGui_ImplVulkan_PipelineInfo{
            .PipelineRenderingCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &color_format,
                .depthAttachmentFormat = depth_format,
            },
        },
        .UseDynamicRendering = true,
        .CheckVkResultFn = [](VkResult err) { detail::debug::vk_check(err); },
    };
    if(! ImGui_ImplVulkan_Init(&info)){
        LOG_FATAL("ImGui_ImplVulkanInit failed.");
    }
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
}

auto record_imgui_commands(vk::raii::CommandBuffer const& cmdBuf) -> void{
    if (auto* draw_data = ImGui::GetDrawData()){
       ImGui_ImplVulkan_RenderDrawData(draw_data,*cmdBuf);
    }
}
