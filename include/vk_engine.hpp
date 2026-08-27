#pragma once 
#include "shared.hpp"
#include "vk_types.hpp"
#include "vk_frame_data.hpp"
#include "vk_swapchain.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_to_string.hpp>

FWD_DECL_STRUCT(SDL_Window);
struct VkEngine {
  public:
    static constexpr u32 syncFrameCount = 2;
    // the 'logical' size of the window. 
    // On a display with HiDPI (eg apple retina), the logical size will always be `x` times smaller than the 
    // 'pixel size', where `x` is the HiDPI pixel ratio.
    static constexpr vk::Extent2D m_windowLogicalSize{800, 600};
    vk::Extent2D get_framebuffer_size() const noexcept;
    static constexpr bool m_useValidationLayers{true};

    VkEngine() {
        init(); 
    }
    ~VkEngine() {
        cleanup(); 
    }
    VkEngine* m_loadedEngine{};

    size_t m_frameCount{};
    bool m_shouldStopRendering{};
    bool m_shouldStopRunning{};
    bool m_framebufferResized{};
    u32 m_vkQueueFamily{};


    SDL_Window* m_window{};

    vk::raii::Context m_vkContext{};
    vk::raii::Instance m_vkInstance{nullptr};
    vk::raii::DebugUtilsMessengerEXT m_vkDebugMessenger{nullptr};
    vk::raii::SurfaceKHR m_vkSurface{nullptr};
    vk::raii::PhysicalDevice m_vkPhysicalDevice{nullptr};
    vk::raii::Device m_vkDevice{nullptr};
    vk::raii::Queue m_vkQueue{nullptr};

    Swapchain m_swapchain{};

    std::array<FrameData, syncFrameCount> m_inflightFrames{};

    vk::raii::PipelineLayout m_vkPipelineLayout{nullptr};
    vk::raii::Pipeline m_vkPipeline{nullptr};

    vk::raii::CommandPool m_commandPool{nullptr};
    void record_commands_to_buffer(u32 imageIndex);

    FrameData& get_current_frame();


    // struct idea:
    struct VBO{
        size_t n_vertices{};
        vk::raii::Buffer buf{nullptr};
        vk::raii::DeviceMemory memory{nullptr};
    };

    static constexpr size_t n_vertices  = 3;
    vk::raii::Buffer vertexBuffer{nullptr};
    vk::raii::DeviceMemory vertexBufferMemory{nullptr};

    VkEngine& get_instance();

    bool is_initialized();

    void init();
    void cleanup();

    void run();
    void draw();
    void handle_inputs();
    

  private:
    void init_window();
    void init_vulkan();
    void init_vtx_data();
    void init_swapchain();
    void init_pipeline();
    void init_commands();
    void init_sync_structures();

    void recreate_swapchain();
    [[nodiscard]] 
    auto make_shader_module(std::span<const char> spirv_src);
    inline auto select_memory_type(u32 type_flags_required, vk::MemoryPropertyFlags prop_flags_required){
        auto device_memory_properties = m_vkPhysicalDevice.getMemoryProperties();
        u32 selected_mem_type_idx{numeric_max<u32>};
        for (u32 idx = 0 ; idx<device_memory_properties.memoryTypeCount; idx++){
            auto const& memoryType  = device_memory_properties.memoryTypes[idx];
            auto const& propertyFlags = memoryType.propertyFlags;

            bool matches_filter = type_flags_required & (1 << idx);
            bool matches_properties = (propertyFlags & prop_flags_required) == prop_flags_required;
            if (!matches_filter){
                LOG_DBG("Memtype: [{}]{} does not match mem_type_filter",idx, vk::to_string(memoryType.propertyFlags));
            }
            if (!matches_properties){
                LOG_DBG("Memtype: [{}]{} does not match property flags ({})",idx, vk::to_string(memoryType.propertyFlags),vk::to_string(prop_flags_required));
            }
            if (matches_filter && matches_properties){
                selected_mem_type_idx = idx;
                break;
            }
        }
        if (selected_mem_type_idx == numeric_max<u32>) {
            LOG_FATAL("Unable to find suitable memory type for buffer creation.");
        }
        return selected_mem_type_idx;
    }
    inline auto make_vertex_buffer( size_t size_bytes, vk::SharingMode sharing_mode ){
        auto buf = vk::raii::Buffer{
            m_vkDevice,
            vk::BufferCreateInfo{
                .size = size_bytes,
                .usage = vk::BufferUsageFlagBits::eVertexBuffer,
                .sharingMode = sharing_mode
            },
        };
        auto mem_requirements = buf.getMemoryRequirements();
        auto memType = select_memory_type(
            mem_requirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
        );
        auto memAllocInfo = vk::MemoryAllocateInfo{
            .allocationSize = mem_requirements.size,
            .memoryTypeIndex = memType,
        };
        auto memory = vk::raii::DeviceMemory{
            m_vkDevice,
            memAllocInfo,
        };
        
        buf.bindMemory(*memory, 0);

        return std::pair{std::move(buf),std::move(memory)};
    }

    // required to extend the lifetime of the object names we use for debugging
    std::vector<std::string> objectNames;
    template <typename T>
    void set_vkobject_dbg_name(T const& object, std::string name_in) {
        vk::DebugUtilsObjectNameInfoEXT nameInfo;
        auto name = objectNames.emplace_back(std::move(name_in));
        
        nameInfo.objectType   = T::objectType; 
        nameInfo.objectHandle = reinterpret_cast<uint64_t>(get_c_handle(object));
        nameInfo.pObjectName  = name.c_str(); // should live for as long as the engine does

        m_vkDevice.setDebugUtilsObjectNameEXT(nameInfo);
    } 
    void cleanup_window() const noexcept;
};
