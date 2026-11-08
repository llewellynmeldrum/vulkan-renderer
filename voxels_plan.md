# plan
1. introduce a new voxel dda pipeline
2. Make a new descriptor set layout for this pipeline, I dont think reusing the same one over and over is the right idea.
3. 



# Other ideas 
- split renderer into GpuContext and FrameLoopContext


GpuContext: Long lived, renderer context, which likely doenst change much. 
FrameLoop: Short lived drawing context, which gets const references to GpuContext.
```cpp
struct FrameLoop{
    u32                                  m_frameCount{0};
    glm::vec2                            m_windowLogicalExtent;
    glm::vec2                            m_windowPixelExtent;
    glm::vec2                            m_viewportOffset {0.0f,0.0f};
    bool                                 m_requiresSwapchainRecreation{};
    std::vector<vk::raii::DescriptorSet> m_vkDescriptorSets; // indexed by frame
    std::vector<FrameData>               m_inflightFrames{};
    vk::PolygonMode                      m_vkPolygonMode {vk::PolygonMode::eFill};
};
struct GpuContext{
    VmaAllocator                         m_allocator;
    SDL_Window*                          m_window{};
    // } 


    vk::raii::Context                    m_vkContext{};
    vk::raii::Instance                   m_vkInstance{nullptr};
    vk::raii::DebugUtilsMessengerEXT     m_vkDebugMessenger{nullptr};
    vk::raii::SurfaceKHR                 m_vkSurface{nullptr};
    u32                                  m_vkQueueFamily{};

    vk::raii::PhysicalDevice             m_vkPhysicalDevice{nullptr};
    vk::raii::Device                     m_vkDevice{nullptr};
    vk::raii::Queue                      m_vkQueue{nullptr};

    Swapchain                            m_swapchain{};

    vk::raii::DescriptorPool             m_vkDescriptorPool{nullptr};
    vk::raii::DescriptorSetLayout        m_vkDescriptorSetLayout{nullptr};

    Texture2D                            m_texture{};
    DepthAttachment                      m_depthImage{nullptr};

    // make the separation
    static constexpr inline auto vk_enabledDynamicState = std::array{
        vk::DynamicState::eViewport, 
        vk::DynamicState::eScissor,
        vk::DynamicState::ePolygonModeEXT,
    };

    Pipeline<PushConstants::ModelMatrix>                m_fill_pipeline{};
    Pipeline<PushConstants::ModelMatrix>                m_line_pipeline{};
    Pipeline<PushConstants::Transform2D>                m_2d_pipeline{};
    Pipeline<PushConstants::Transform2D>                m_voxel_dda_pipeline{};
};
```
