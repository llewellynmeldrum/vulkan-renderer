#include "vk_engine.hpp"
#include <thread>
void VkEngine::recreate_swapchain() {
    using namespace std::chrono_literals;
    auto framebuffer_sz = get_framebuffer_size();
    while (framebuffer_sz.width == 0 || framebuffer_sz.height == 0){
        // window is minimized 
        std::this_thread::sleep_for(10ms);
        handle_inputs();
        framebuffer_sz = get_framebuffer_size();
    }
    m_vkDevice.waitIdle();
    m_swapchain.descriptor.clear();
    init_swapchain();
}
void VkEngine::copy_buffer(vk::raii::Buffer const & src, vk::raii::Buffer &dst, vk::DeviceSize size){
    auto allocInfo = vk::CommandBufferAllocateInfo{
        .commandPool = *get_current_frame().commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };

    auto cmdCopyBuf = std::move(m_vkDevice.allocateCommandBuffers(allocInfo).front());

    cmdCopyBuf.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    cmdCopyBuf.copyBuffer(src,dst,vk::BufferCopy{.srcOffset=0, .dstOffset = 0, .size=size});
    cmdCopyBuf.end();

    m_vkQueue.submit(
        vk::SubmitInfo{}
            .setCommandBuffers(*cmdCopyBuf),
        nullptr
    );
    // We wait to ensure the transfer happened
    m_vkQueue.waitIdle();
}
