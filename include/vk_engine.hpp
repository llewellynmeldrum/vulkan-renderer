#pragma once 
#include "shared.hpp"
#include "vk_types.hpp"
#include "camera.hpp"
#include "vk_frame_data.hpp"
#include "vk_swapchain.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_to_string.hpp>


#include "vk_debug.hpp"

FWD_DECL_STRUCT(SDL_Window);
FWD_DECL_STRUCT(SDL_KeyboardEvent);
struct VkEngine {
  public:
    static constexpr u32 syncFrameCount = 2;
    // the 'logical' size of the window. 
    // On a display with HiDPI (eg apple retina), the logical size will always be `x` times smaller than the 
    // 'pixel size', where `x` is the HiDPI pixel ratio.
    static constexpr vk::Extent2D m_windowLogicalSize{800, 600};
    static constexpr u32 API_VER = vk::ApiVersion13;


    vk::Extent2D get_framebuffer_size() const noexcept;
    static constexpr bool m_useValidationLayers{true};

    VkEngine() { init(); }
    ~VkEngine() { cleanup(); }

    VkEngine* m_loadedEngine{};

    size_t m_frameCount{};
    bool m_shouldStopRendering{};
    bool m_shouldStopRunning{};
    bool m_framebufferResized{};
    u32 m_vkQueueFamily{};

    Camera m_cam;

    SDL_Window* m_window{};

    vk::raii::Context m_vkContext{};
    vk::raii::Instance m_vkInstance{nullptr};
    vk::raii::DebugUtilsMessengerEXT m_vkDebugMessenger{nullptr};
    vk::raii::SurfaceKHR m_vkSurface{nullptr};
    vk::raii::PhysicalDevice m_vkPhysicalDevice{nullptr};
    vk::raii::Device m_vkDevice{nullptr};
    vk::raii::Queue m_vkQueue{nullptr};

    Swapchain m_swapchain{};

    std::vector<FrameData> m_inflightFrames{};

    vk::raii::DescriptorSetLayout m_vkDescriptorSetLayout{nullptr};
    vk::raii::DescriptorPool m_vkDescriptorPool{nullptr};
    std::vector<vk::raii::DescriptorSet> m_vkDescriptorSets;
    vk::raii::PipelineLayout m_vkPipelineLayout{nullptr};
    vk::raii::Pipeline m_vkPipeline{nullptr};

    void record_commands_to_buffer(u32 imageIndex);

    static constexpr inline auto vk_enabledDynamicState = std::array{
        vk::DynamicState::eViewport, 
        vk::DynamicState::eScissor,
        vk::DynamicState::ePolygonModeEXT,
    };
    void set_dynamic_state(vk::raii::CommandBuffer const& cmdBuf);
    vk::PolygonMode m_vkPolygonMode {vk::PolygonMode::eFill};
    FrameData& get_current_frame();
    u32 get_current_frame_index();


    vk::raii::Buffer m_vertexBuffer{nullptr};
    vk::raii::DeviceMemory m_vertexBufferMemory{nullptr};

    vk::raii::Buffer m_indexBuffer{nullptr};
    vk::raii::DeviceMemory m_indexBufferMemory{nullptr};

    VkEngine& get_instance();

    bool is_initialized();

    void init();
    void cleanup();

    void run();
    void draw();
    void handle_key_down(SDL_KeyboardEvent const& key_ev);
    void handle_inputs();
    

  private:
    void init_window();
    void init_vk_instance();
    void init_vk_surface();
    void init_vk_device_and_queue();
    void init_buffers();
    void init_swapchain();
    void init_pipeline();
    void init_descriptor_set_layout();
    void init_descriptor_pool();
    void init_descriptor_sets();
    void init_inflightFrames();
    void init_textures();
    void init_sync_structures();

    void update_uniforms(FrameData const& frame);
    void copy_buffer(vk::raii::Buffer const & src, vk::raii::Buffer &dst, vk::DeviceSize size);
    void recreate_swapchain();
    auto dyn_get_viewport() const{
        return vk::Viewport{
            0.0f,0.0f, // viewport position
            st_cast<f32>(m_swapchain.extent.width), st_cast<f32>(m_swapchain.extent.height),
            0.0f, 1.0f // min and max depth
        };
    }
    auto dyn_get_scissor() const{
        return vk::Rect2D{
            vk::Offset2D{0,0},
            m_swapchain.extent
        };
    }
    auto dyn_get_polymode() const{
        return m_vkPolygonMode;
    }

    void cleanup_window() const noexcept;
};
