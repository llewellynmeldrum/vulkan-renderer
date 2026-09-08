#include "renderer.hpp"
auto 
Renderer::end_single_use_cmd(vk::raii::CommandBuffer&& cmdBuf)
-> void
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
-> vk::raii::CommandBuffer
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
