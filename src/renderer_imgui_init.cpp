#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_sdl3.h>
#include <vulkan/vulkan_core.h>

#include "renderer.hpp"
#include "vk_types.hpp"

void Renderer::init_imgui() {
    using detail::debug::get_c_handle;
    auto log_failure = [](std::string msg){
        LOG_FATAL("Failed to initialize imgui : {}",msg);
    };
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    if(! ImGui_ImplSDL3_InitForVulkan(m_window)){
        log_failure("ImGui_ImplSDL3_InitForVulkan failed.");
    }


    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.ScaleAllSizes(k_guiMainScale);
    style.FontScaleDpi = k_guiMainScale;


    auto const color_format = static_cast<VkFormat>(m_swapchain.imageFormat);
    auto const depth_format = static_cast<VkFormat>(DepthAttachment::select_depth_format());

    auto info = ImGui_ImplVulkan_InitInfo{
        .ApiVersion = k_API_VER,
        .Instance = get_c_handle(m_vkInstance),
        .PhysicalDevice = get_c_handle(m_vkPhysicalDevice),
        .Device = get_c_handle(m_vkDevice),
        .QueueFamily = m_vkQueueFamily,
        .Queue = get_c_handle(m_vkQueue),
        .DescriptorPoolSize = 64,
        .MinImageCount = k_syncFrameCount,
        .ImageCount = k_syncFrameCount,
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
        log_failure("ImGui_ImplVulkanInit failed.");
    }
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;

}
void Renderer::record_imgui_commands(vk::raii::CommandBuffer const& cmdBuf){
    if (auto* draw_data = ImGui::GetDrawData()){
       ImGui_ImplVulkan_RenderDrawData(draw_data,*cmdBuf);
    }
}
void Renderer::cleanup_imgui(){
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
