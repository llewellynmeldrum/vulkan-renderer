#pragma once 
#include <unordered_map>

#include "renderer2d.hpp"
#include "shared_transformations.hpp"
#include "renderer_types.hpp"
#include "mesh_id.hpp"

FWD_DECL_STRUCT(SDL_Window);
FWD_DECL_STRUCT(SDL_KeyboardEvent);
struct Renderer {
  public:
    static constexpr auto                k_shader_spirv_path = "shaders/slang.spv"sv;
    static constexpr u32                 k_syncFrameCount = 2;
    static constexpr u32                 k_API_VER = vk::ApiVersion13;
    static constexpr bool                k_useValidationLayers{true};


    Renderer2D                          m_rend2d;
    // handles 
    VmaAllocator                         m_allocator;
    SDL_Window*                          m_window{};

    // primitive types
    u32                                  m_frameCount{0};
    glm::vec2                            m_windowLogicalExtent;
    glm::vec2                            m_windowPixelExtent;
    bool                                 m_requiresSwapchainRecreation{};
    glm::vec2                            m_viewportOffset {0.0f,0.0f};

    // vulkan types

    // 3d mesh soas 
    std::unordered_map<MeshID, GpuMesh3D>     m_gpu_meshes3d;
    std::unordered_map<MeshID, glm::mat4x4> m_model_matrices;

    auto upload_mesh2d() -> void;
    auto upload_mesh3d(
        MeshID id,
        CpuMesh3D cpu_mesh,
        glm::mat4x4 model_matrix = glm::mat4x4(1.0f)
    ) -> void ;

    auto make_gpu_mesh(CpuMesh2D const& cpu_mesh) -> GpuMesh2D;
    auto make_gpu_mesh(CpuMesh3D const& cpu_mesh) -> GpuMesh3D;

    template<typename tVertexType, typename tIndexType>
    auto make_gpu_mesh_impl(CpuMesh<tVertexType,tIndexType> const& cpu_mesh) -> GpuMesh<tVertexType,tIndexType>;

    u32                                  m_vkQueueFamily{};

    vk::raii::Context                    m_vkContext{};
    vk::raii::Instance                   m_vkInstance{nullptr};
    vk::raii::DebugUtilsMessengerEXT     m_vkDebugMessenger{nullptr};
    vk::raii::SurfaceKHR                 m_vkSurface{nullptr};

    vk::raii::PhysicalDevice             m_vkPhysicalDevice{nullptr};
    vk::raii::Device                     m_vkDevice{nullptr};
    vk::raii::Queue                      m_vkQueue{nullptr};

    Swapchain                            m_swapchain{};
    std::vector<FrameData>               m_inflightFrames{};

    vk::raii::DescriptorPool             m_vkDescriptorPool{nullptr};
    vk::raii::DescriptorSetLayout        m_vkDescriptorSetLayout{nullptr};
    std::vector<vk::raii::DescriptorSet> m_vkDescriptorSets; // indexed by frame

    Texture2D                            m_texture{nullptr};
    DepthAttachment                      m_depthImage{nullptr};

    ShaderPipelineContext                m_fill_pipeline{};
    ShaderPipelineContext                m_line_pipeline{};
    ShaderPipelineContext                m_2d_pipeline{};

    // dynamic vulkan state
    vk::PolygonMode                      m_vkPolygonMode {vk::PolygonMode::eFill};
    static constexpr inline auto vk_enabledDynamicState = std::array{
        vk::DynamicState::eViewport, 
        vk::DynamicState::eScissor,
        vk::DynamicState::ePolygonModeEXT,
    };

    auto
    get_framebuffer_size() const noexcept
    -> vk::Extent2D;

    auto toggle_wireframe()
    -> void;


    auto prepare_render_attachments(
        vk::raii::CommandBuffer const& cmdBuf,
        u32 imageIndex, 
        std::array<f32, 4> clearColor
    )
    -> RenderAttachmentContext;



    auto record_commands(
        Camera const& cam,
        FrameData const& frame, u32 imageIndex
    ) -> void;

    auto set_dynamic_state(vk::raii::CommandBuffer const& cmdBuf) 
    -> void;


    auto& get_current_frame(this auto& self) {
        return self.m_inflightFrames.at(self.get_current_frame_index());
    }

    [[nodiscard]]
    auto get_current_frame_index() 
    const -> u32;

    auto is_initialized() 
    -> bool;

    auto init(
        SDL_Window* window,
        glm::uvec2 window_extent
    ) 
    -> void;

    auto cleanup() 
    -> void;

    auto draw(Camera const& cam) 
    -> void;

    auto present_image(u32 imageIndex, vk::Result acquire_res) 
    -> void;

    

    auto handle_window_resize(glm::vec2 new_logical_extent) 
    -> void;
        
    auto 
    copy_buffer(
        vk::Buffer const & src,
        vk::Buffer const& dst,
        vk::DeviceSize size,
        vk::DeviceSize offset=0
    ) const -> void;
  private:
    auto prepare_pass(
        vk::raii::CommandBuffer const& cmdBuf,
        ShaderPipelineContext const& pipeline, 
        u32 frameIndex, 
        vk::PolygonMode poly_mode
    ) -> void;

    auto recreate_swapchain() -> void;

    auto init_vulkan() -> void;

    auto init_fill_pipeline(vk::raii::ShaderModule const& shader_module) -> void;
    auto init_line_pipeline(vk::raii::ShaderModule const& shader_module) -> void;
    auto init_2d_pipeline(vk::raii::ShaderModule const& shader_module) -> void;

    auto init_texture() -> void;

    auto init_sync_structures() -> void;

    auto upload_mesh_data(
        std::span<const GpuMesh3D::VertexType> in_vertices,
        std::span<const GpuMesh3D::IndexType> in_indices
    ) -> void;

    // drawing shit
    auto 
    update_ubo(
        FrameData const& frame,
        Camera const& cam,
        glm::mat4x4 model_matrix
    ) -> void;


    auto
    begin_single_use_cmd() 
    const -> vk::raii::CommandBuffer;

    auto 
    end_single_use_cmd(vk::raii::CommandBuffer&& cmdBuf) 
    const -> void;

    auto get_viewport() const
    -> vk::Viewport;

    auto get_scissor() const
    -> vk::Rect2D;

private:
    static constexpr f32 k_depthMin {0.0f};
    static constexpr f32 k_depthMax {1.0f};
    static constexpr f32 k_depthFar {0.0f};
    static constexpr f32 k_depthNear{1.0f};

    f32 m_pixelSize;
    void cleanup_vma() const noexcept;
    glm::vec2 logical_to_ndc(glm::vec2 pLogical) const{ return detail::logical_to_ndc(pLogical, m_windowLogicalExtent);}
    glm::vec2 ndc_to_logical(glm::vec2 pNdc) const{ return detail::ndc_to_logical(pNdc, m_windowLogicalExtent);}
    glm::vec2 logical_to_pixel(glm::vec2 pLogical) const{ return detail::logical_to_pixel(pLogical,m_pixelSize);}
    glm::vec2 pixel_to_logical(glm::vec2 pPixel) const{ return detail::pixel_to_logical(pPixel,m_pixelSize);}
};

