#pragma once 
#include "SDL3/SDL_events.h"
#include "Texture2D.hpp"
#include "shared.hpp"
#include "vk_engine_input_keys.hpp"
#include "vk_managed_buffers.hpp"
#include "vk_types.hpp"
#include "camera.hpp"
#include "vk_frame_data.hpp"
#include "vk_swapchain.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_to_string.hpp>


#include "vk_debug.hpp"
#include "cpu_mesh.hpp"
#include "gpu_mesh.hpp"
#include "heightmap.hpp"
#include "depth_image.hpp"

#include "vk_engine_contexts.hpp"

FWD_DECL_STRUCT(SDL_Window);
FWD_DECL_STRUCT(SDL_KeyboardEvent);
struct VkEngine {
  public:
    static constexpr std::string_view shader_src_path = "shaders/slang.spv";
    static constexpr u32 syncFrameCount = 2;
    // the 'logical' size of the window. 
    // On a display with HiDPI (eg apple retina), the logical size will always be `x` times smaller than the 
    // 'pixel size', where `x` is the HiDPI pixel ratio.
    static constexpr vk::Extent2D m_windowLogicalSize{1280, 720};
    static constexpr u32 API_VER = vk::ApiVersion13;


    vk::Extent2D get_framebuffer_size() const noexcept;
    static constexpr bool m_useValidationLayers{true};

    VkEngine() { init(); }
    ~VkEngine() { cleanup(); }


    Heightmap m_heightmap{};
    CpuMesh m_cpu_heightmapMesh{};
    GpuMesh m_gpu_heightmapMesh{};


    VmaAllocator m_allocator;

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

    vk::raii::DescriptorPool m_vkDescriptorPool{nullptr};

    vk::raii::DescriptorSetLayout m_vkDescriptorSetLayout{nullptr};
    std::vector<vk::raii::DescriptorSet> m_vkDescriptorSets; // indexed by frame

    ShaderPipelineContext m_fill_pipeline{};
    ShaderPipelineContext m_line_pipeline{};

    RenderAttachmentContext prepare_render_attachments(
        vk::raii::CommandBuffer const& cmdBuf,
        u32 imageIndex,
        std::array<f32, 4> clearColor
    );

    void draw_mesh(vk::raii::CommandBuffer const& cmdBuf, GpuMesh const& gpu_mesh);

    void record_commands(FrameData const& frame, u32 imageIndex);

    static constexpr inline auto vk_enabledDynamicState = std::array{
        vk::DynamicState::eViewport, 
        vk::DynamicState::eScissor,
        vk::DynamicState::ePolygonModeEXT,
    };
    void set_dynamic_state(vk::raii::CommandBuffer const& cmdBuf);
    vk::PolygonMode m_vkPolygonMode {vk::PolygonMode::eFill};
    FrameData& get_current_frame();
    u32 get_current_frame_index();



    Texture2D m_texture{nullptr};
    DepthAttachment m_depthImage{nullptr};



    bool is_initialized();

    void init();
    void cleanup();

    void run();
    void per_frame_update();
    void draw();
    void draw_pass( vk::raii::CommandBuffer const& cmdBuf, ShaderPipelineContext const&  ctx, u32 frameIndex, vk::PolygonMode poly_mode);
    void present_image(u32 imageIndex, vk::Result acquire_res);
    void handle_inputs();
    

  private:
    void init_sdl();
    void init_vulkan();
    void init_fill_pipeline(vk::raii::ShaderModule const& shader_module);
    void init_line_pipeline(vk::raii::ShaderModule const& shader_module);
    void init_texture();
    void init_sync_structures();
    void init_heightmap();
    void upload_heightmap();



    GpuMesh upload_gpu_mesh(CpuMesh const& cpu_mesh);

    // drawing shit
    void update_uniforms(FrameData const& frame);
    void copy_buffer(vk::Buffer const & src, vk::Buffer const& dst, vk::DeviceSize size, vk::DeviceSize offset=0);
    void recreate_swapchain();
    auto begin_single_use_cmd() -> vk::raii::CommandBuffer;
    void end_single_use_cmd(vk::raii::CommandBuffer&& cmdBuf);



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


    void process_inputs(SDL_KeyboardEvent const& ev);
    void handle_mouse_motion(SDL_MouseMotionEvent const& ev);
    void handle_scroll_motion(SDL_MouseWheelEvent const& ev);

    // cleanup 
    void cleanup_window() const noexcept;
    void cleanup_vma() const noexcept;
    std::array<bool, KeyCode::COUNT> keystate{};
    std::array<bool, KeyCode::COUNT> keys_pressed_last_frame{};
    bool is_down(int key){
        return keystate[key];
    }

    bool just_pressed(int key){
        bool pressed_last_frame = keys_pressed_last_frame[key];
        bool pressed_this_frame = keystate[key];

        if (pressed_this_frame){
            LOG_DBG("Key {} was pressed this frame, ",key);
        }
        if (pressed_last_frame){
            LOG_DBG("And was     pressed last frame. ");
        }else{
            LOG_DBG("And was NOT pressed last frame. ");
        }
        return !pressed_last_frame && pressed_this_frame;
    }
};

