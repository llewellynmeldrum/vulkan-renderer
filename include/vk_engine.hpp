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
    void init_sync_structures();

    void update_uniforms(FrameData const& frame);
    void copy_buffer(vk::raii::Buffer const & src, vk::raii::Buffer &dst, vk::DeviceSize size);
    void recreate_swapchain();
    [[nodiscard]] 
    auto make_shader_module(std::span<const char> spirv_src);
    [[nodiscard]]
    inline auto select_memory_type(u32 type_flags_required, vk::MemoryPropertyFlags prop_flags_required){
        auto device_memory_properties = m_vkPhysicalDevice.getMemoryProperties();
        u32 selected_mem_type_idx{numeric_max<u32>};
        for (u32 idx = 0 ; idx<device_memory_properties.memoryTypeCount; idx++){
            auto const& memoryType  = device_memory_properties.memoryTypes[idx];
            auto const& propertyFlags = memoryType.propertyFlags;

            bool matches_filter = type_flags_required & (1 << idx);
            bool matches_properties = (propertyFlags & prop_flags_required) == prop_flags_required;
            if (!matches_filter){
 //               LOG_DBG("Memtype: [{}] does not match mem_type_filter",idx);
            }
            if (!matches_properties){
//                LOG_DBG("Memtype: [{}]{} does not match property flags ({})",idx, vk::to_string(memoryType.propertyFlags),vk::to_string(prop_flags_required));
            }
            if (matches_filter && matches_properties){
                selected_mem_type_idx = idx;
                break;
            }
        }
        if (selected_mem_type_idx == numeric_max<u32>) {
            LOG_FATAL("Unable to find suitable memory type for buffer creation.");
        }else{
            auto const& memoryType  = device_memory_properties.memoryTypes[selected_mem_type_idx];
            LOG_DBG(
                "Selected Memtype: [{}]{} for ({})",
                selected_mem_type_idx, 
                vk::to_string(memoryType.propertyFlags),
                vk::to_string(prop_flags_required)
            );
        }
        return selected_mem_type_idx;
    }
    [[nodiscard]]
    inline auto make_buffer( size_t size_bytes,  vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memFlags){
        auto buf = vk::raii::Buffer{
            m_vkDevice,
            {
                .size = size_bytes,
                .usage = usage,
                .sharingMode = vk::SharingMode::eExclusive,
            },
        };
        auto mem_requirements = buf.getMemoryRequirements();
        auto memType = select_memory_type(
            mem_requirements.memoryTypeBits,
            memFlags
        );
        auto memory = vk::raii::DeviceMemory{
            m_vkDevice,
            {
                .allocationSize = mem_requirements.size,
                .memoryTypeIndex = memType,
            },
        };
        
        buf.bindMemory(*memory, 0);

        return std::pair{std::move(buf),std::move(memory)};
    }
    [[nodiscard]]
    inline auto make_vertex_buffer( size_t size_bytes, vk::SharingMode sharing_mode ){
        return make_buffer(
            size_bytes,
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );
    }
    [[nodiscard]]
    inline auto make_uniform_buffer( size_t size_bytes, vk::SharingMode sharing_mode ){
        return make_buffer(
            size_bytes,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal
        );

    }
    [[nodiscard]]
    inline auto make_index_buffer( size_t size_bytes, vk::SharingMode sharing_mode ){
        return make_buffer(
            size_bytes,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );
    }
    [[nodiscard]]
    inline auto make_staging_buffer( size_t size_bytes){
        return make_buffer(
            size_bytes,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );
    }
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
