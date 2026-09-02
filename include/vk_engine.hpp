#pragma once 
#include "SDL3/SDL_events.h"
#include "shared.hpp"
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

    vk::raii::DescriptorSetLayout m_vkDescriptorSetLayout{nullptr};
    vk::raii::DescriptorPool m_vkDescriptorPool{nullptr};
    std::vector<vk::raii::DescriptorSet> m_vkDescriptorSets; // indexed by frame
    vk::raii::PipelineLayout m_vkPipelineLayout{nullptr};
    vk::raii::Pipeline m_vkPipeline{nullptr};

    struct RenderAttachments{
        vk::RenderingAttachmentInfo colorAttachment;
        vk::RenderingAttachmentInfo depthAttachment;
        auto get_info(Swapchain const& swapchain) const{
            return vk::RenderingInfo{}
                .setRenderArea({.offset={},.extent=swapchain.extent})
                .setLayerCount(1)
                .setColorAttachments(colorAttachment)
                .setPDepthAttachment(&depthAttachment)
            ;
        }

    };
    RenderAttachments prepare_render_attachments(vk::raii::CommandBuffer const& cmdBuf, u32 imageIndex, std::array<f32, 4> clearColor);
    void draw_mesh(vk::raii::CommandBuffer const& cmdBuf, GpuMesh const& gpu_mesh);
    void record_commands(vk::raii::CommandBuffer const& cmdBuf, u32 imageIndex);

    static constexpr inline auto vk_enabledDynamicState = std::array{
        vk::DynamicState::eViewport, 
        vk::DynamicState::eScissor,
        vk::DynamicState::ePolygonModeEXT,
    };
    void set_dynamic_state(vk::raii::CommandBuffer const& cmdBuf);
    vk::PolygonMode m_vkPolygonMode {vk::PolygonMode::eFill};
    FrameData& get_current_frame();
    u32 get_current_frame_index();



    vk::raii::Image m_vkTextureImage{nullptr};
    vk::raii::DeviceMemory m_vkTextureImageMemory{nullptr};
    vk::raii::ImageView m_vkTextureImageView{nullptr};
    vk::raii::Sampler m_vkTextureSampler{nullptr};

    vk::raii::Image m_vkDepthImage{nullptr};
    vk::raii::DeviceMemory m_vkDepthImageMemory{nullptr};
    vk::raii::ImageView m_vkDepthImageView{nullptr};

    VkEngine& get_instance();

    bool is_initialized();

    void init();
    void cleanup();

    void run();
    void draw();
    void present_image(u32 imageIndex, vk::Result acquire_res);
    void handle_inputs();
    

  private:
    void init_sdl();
    void init_vk_instance();
    void init_vma();
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
    void init_depth_attachment();
    void init_sync_structures();
    void init_heightmap();
    void upload_heightmap();



    GpuMesh upload_gpu_mesh(CpuMesh const& cpu_mesh);

    // drawing shit
    void update_uniforms(FrameData const& frame);
    void copy_buffer(vk::Buffer const & src, vk::Buffer const& dst, vk::DeviceSize size, vk::DeviceSize offset=0);
    void recreate_swapchain();
    vk::raii::CommandBuffer begin_single_use_cmd();
    void end_single_use_cmd(vk::raii::CommandBuffer&& cmdBuf);

    void copy_buffer_to_image(
        vk::raii::CommandBuffer const& cmd, 
        vk::raii::Buffer const& src_buffer, 
        vk::raii::Image const& dst_image,
        vk::Extent2D img_extent
    );
    vk::Format select_depth_format(){
        // TODO: if bothered, make a selector here, since some devices dont support it 
        return vk::Format::eD32Sfloat;
    }

    void transition_img_layout(
        vk::raii::CommandBuffer const& cmd, 
        vk::raii::Image const& img,
        vk::ImageLayout old_layout,
        vk::ImageLayout new_layout
        ,vk::ImageAspectFlags image_aspect_flags
    );

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


    void handle_key_down(SDL_KeyboardEvent const& ev);
    void handle_mouse_motion(SDL_MouseMotionEvent const& ev);
    void handle_scroll_motion(SDL_MouseWheelEvent const& ev);

    // cleanup 
    void cleanup_window() const noexcept;
    void cleanup_vma() const noexcept;
};
