#include "renderer.hpp"
auto 
Renderer::end_single_use_cmd(vk::raii::CommandBuffer&& cmdBuf)
const -> void
{
    cmdBuf.end();
    m_vkQueue.submit(
        vk::SubmitInfo{}
            .setCommandBuffers(*cmdBuf)
    );
    m_vkQueue.waitIdle();
}

auto 
Renderer::begin_single_use_cmd() 
const -> vk::raii::CommandBuffer
{
    auto cmdBuf =  std::move(vk::raii::CommandBuffers(
            m_vkDevice,
            vk::CommandBufferAllocateInfo{}
                .setCommandPool(get_current_frame().commandPool)
                .setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandBufferCount(1)
        ).front());
    ;
    cmdBuf.begin(
        vk::CommandBufferBeginInfo{}
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
    );
    return cmdBuf;
}

namespace detail{
auto perform_extra_pipeline_check(PipelineCreateInfo const& info) -> void {
        auto str_contains = [](std::string const& file_contents, std::string_view search_term){
            return file_contents.find(search_term) != std::string::npos;
        };
        auto cmake_file = read_file_contents<std::string>("./CMakeLists.txt");
        ASSERT(
            str_contains(cmake_file, info.vertex_fn_name), 
            "Vertex function not found in CMakeLists.txt, did you forget to add it as a entry point?.",
            info.pipeline_name,
            info.vertex_fn_name
        );
        ASSERT(
            str_contains(cmake_file, info.frag_fn_name), 
            "Fragment function not found in CMakeLists.txt, did you forget to add it as a entry point?.",
            info.pipeline_name,
            info.frag_fn_name
        );
        ASSERT(
            str_contains(cmake_file, info.shader_module_name), 
            "Shader module name not found in CMakeLists.txt, check `add_slang_shader_target()` invocations.",
            info.pipeline_name,
            info.shader_module_name
        );
}
auto make_shader_stages(PipelineCreateInfo const& info) 
-> std::array<vk::PipelineShaderStageCreateInfo,2>{
    auto vtxStageInfo = vk::PipelineShaderStageCreateInfo{}
        .setStage(vk::ShaderStageFlagBits::eVertex)
        .setModule(info.shader_module)
        .setPName(info.vertex_fn_name.c_str())
    ;
    auto fragStageInfo = vk::PipelineShaderStageCreateInfo{}
        .setStage(vk::ShaderStageFlagBits::eFragment)
        .setModule(info.shader_module)
        .setPName(info.frag_fn_name.c_str())
    ;
    return {
        vtxStageInfo,
        fragStageInfo,
    };
}

auto get_raster_state(PipelineCreateInfo const& info)
-> vk::PipelineRasterizationStateCreateInfo{
    static constexpr auto k_front_face_cull_mode = vk::FrontFace::eClockwise;
    return vk::PipelineRasterizationStateCreateInfo{}
        .setDepthClampEnable        (vk::False)
        .setRasterizerDiscardEnable (vk::False)
        .setPolygonMode             (info.poly_mode)
        .setCullMode                (info.cull_mode) 
        .setFrontFace               (k_front_face_cull_mode)
        .setDepthBiasEnable         (info.depth_bias)
        .setDepthBiasConstantFactor (info.depth_bias_constant)
        .setDepthBiasSlopeFactor    (info.depth_bias_slope)
        .setLineWidth               (1.0f) // macs dont support anything higher, no point parametizing this
    ;
}

auto make_depth_stencil_state (PipelineCreateInfo const& info)
-> vk::PipelineDepthStencilStateCreateInfo{
        return vk::PipelineDepthStencilStateCreateInfo{}
        .setDepthTestEnable(info.depth_test)
        .setDepthWriteEnable(info.depth_write)
        .setDepthCompareOp(info.depth_compar)
        .setDepthBoundsTestEnable(vk::False)
        .setStencilTestEnable(vk::False)
    ;
};
}// namespace detail
