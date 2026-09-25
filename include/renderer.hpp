#pragma once 
#include <unordered_map>

#include "byte_span.hpp"
#include "push_constants.hpp"
#include "renderer2d.hpp"
#include "renderer_buffer_helpers.hpp"
#include "shared_transformations.hpp"
#include "renderer_types.hpp"
#include "mesh_id.hpp"
#include "vk_image_helpers.hpp"

#include <stb_image_write.hpp>
#include <stb_rect_pack.hpp>
#include <stb_truetype.hpp>


FWD_DECL_STRUCT(SDL_Window);
FWD_DECL_STRUCT(SDL_KeyboardEvent);
struct Renderer {
  public:
    static constexpr auto                k_shader_spirv_path = "shaders/bin/main_shaders.spv"sv;
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

    Texture2D                            m_texture{};
    DepthAttachment                      m_depthImage{nullptr};

    Pipeline<PushConstants::ModelMatrix>                m_fill_pipeline{};
    Pipeline<PushConstants::ModelMatrix>                m_line_pipeline{};
    Pipeline<PushConstants::Transform2D>         m_2d_pipeline{};

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
//    auto prepare_pass(
//        vk::raii::CommandBuffer const& cmdBuf,
//        Pipeline const& pipeline, 
//        u32 frameIndex
//    ) -> void;

    auto recreate_swapchain() -> void;

    auto init_vulkan() -> void;

    auto init_fill_pipeline(detail::helpers::ShaderModuleWrapper const& shader) -> void;
    auto init_line_pipeline(detail::helpers::ShaderModuleWrapper const& shader) -> void;
    auto init_2d_pipeline  (detail::helpers::ShaderModuleWrapper const& shader) -> void;


    // invokes record_fn(cmdBuf) on a temporary command buf, created and destroyed automatically.
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
        Camera const& cam
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
    auto make_texture_2d(
        ImageData const& img_data,
        vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal
    ) -> Texture2D{
        auto res = Texture2D{};
        res.layout = layout;
        res.textureImage = upload_texture_image(img_data);
        res.image_view = Texture2D::make_view(res.textureImage, img_data.meta.vk_format, m_vkDevice);
        res.sampler = Texture2D::make_sampler(m_vkDevice,m_vkPhysicalDevice);
        res.descriptor_image_info = Texture2D::make_descriptor_image_info(res.sampler,res.image_view, res.layout);
        return res;
    }

    auto make_texture_2d(
        std::string_view file_name,
        vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal
    ) -> Texture2D{
        return make_texture_2d(
            ImageData::from_filename(file_name),
            layout
        );
    }

    auto upload_texture_image(
        ImageData const& img_raw_data
    )-> AllocatedImage {
        LOG_DBG("[{}] - uploadImage  :  BEGIN.",img_raw_data.meta.file_name);
        ASSERT(img_raw_data.buf.size() > 0, "Vulkan does not support buffers of 0 bytes without extensions");
        auto stagingBuffer = AllocatedBuffer(m_allocator,
            vk::BufferCreateInfo{}
                .setSize(img_raw_data.image_size_bytes())
                .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
                .setSharingMode(vk::SharingMode::eExclusive)
            ,
            VmaAllocationCreateInfo{
                .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO
            },
            "Texture image Staging buf"
        );
        LOG_DBG("[{}] - uploadImage  :  Allocated staging buffer ({})",img_raw_data.meta.file_name,img_raw_data.buf.size());
        ASSERT(stagingBuffer.mapped_ptr != nullptr);
        // 2. Copy the data into the staging buffer

        auto image_cpu_to_staging_res = VkResult{};
        LOG_DBG("image raw data size: {}",img_raw_data.image_size_bytes());
        image_cpu_to_staging_res = vmaCopyMemoryToAllocation(
            m_allocator,
            img_raw_data.buf.data(),
            stagingBuffer.allocation,
            0,
            vk::DeviceSize{img_raw_data.image_size_bytes()}
        );
        if (image_cpu_to_staging_res != VkResult::VK_SUCCESS){
            LOG_FATAL("Failed to copy cpu image data into staging buffer.");
        }
        LOG_DBG("uploadImage :  Uploaded image data to staging buffer");

        // 2. create the texture image
        auto textureImage = AllocatedImage(m_allocator,
            vk::ImageCreateInfo{}
                .setImageType(vk::ImageType::e2D)
                .setFormat(img_raw_data.meta.vk_format)
                .setExtent({static_cast<u32>(img_raw_data.meta.px_w),static_cast<u32>(img_raw_data.meta.px_h),1})
                .setMipLevels(1)
                .setArrayLayers(1)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setTiling(vk::ImageTiling::eOptimal)
                .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
                .setSharingMode(vk::SharingMode::eExclusive)
            ,
            VmaAllocationCreateInfo{
                .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
            },
            "Texture image buffer"
        );
        LOG_DBG("Texture2D.uploadImage :  Created allocated image");

        auto cmd = begin_single_use_cmd();
        transition_img_layout(
            cmd,
            textureImage.image, 
            vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
            vk::ImageAspectFlagBits::eColor
        );
        copy_buffer_to_image(cmd,stagingBuffer.buffer, textureImage.image, img_raw_data.get_extent2d());
        transition_img_layout(
            cmd,
            textureImage.image, 
            vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::ImageAspectFlagBits::eColor
        );
        end_single_use_cmd(std::move(cmd));
        LOG_DBG("uploadImage :  Copied staging buffer data to allocated image.");
        return textureImage;
    }
    auto make_font_atlas(std::string_view file_path) 
    -> FontAtlas{
        auto file_contents = read_file_contents(file_path);
        auto font_file_data_raw_span = std::as_writable_bytes(std::span{file_contents});
        stbtt_fontinfo font_info;
        ByteSpan font_file_data{font_file_data_raw_span};
        stbtt_InitFont(
            &font_info, 
            font_file_data.get_ptr_as<uchar const>(),
            stbtt_GetFontOffsetForIndex(
                font_file_data.get_ptr_as<uchar const>(),
                0 
            )
        );
        u32 num_fonts = stbtt_GetNumberOfFonts(font_file_data.get_ptr_as<uchar const>());

        ASSERT(num_fonts != -1, "Font file failed to load properly.");
        ASSERT(num_fonts == 1, "Font file contains more than one font.");

        auto get_glpyh_width = [&](i32 codepoint, f32 font_height){
            f32 scale = stbtt_ScaleForPixelHeight(&font_info, font_height);
            i32 advance_width{0}, left_side_bearing{0};
            stbtt_GetCodepointHMetrics(&font_info, codepoint, &advance_width, &left_side_bearing);
            return advance_width * scale;
        };
        auto const expected_width = get_glpyh_width('X', 1.0f);
        for (char glyph = FontAtlas::code_point_begin_idx; glyph < FontAtlas::code_point_end_idx; glyph++){
            ASSERT(get_glpyh_width(glyph,1.0f) == expected_width, "Font must be monospaced across accepted glyphs");
        }

        // Vector indexed by atlas_size_idx, containing packedchar's indexed by code_point. 
        // i.e packed_quads_vec_per_size[i][j] = the packedchar for size i and code point j.
        auto packed_quads_per_size = std::vector(
            FontAtlas::atlas_size_count,
            std::vector<stbtt_packedchar>(
                FontAtlas::code_point_count,
                stbtt_packedchar{}
            )
        );

        
        // 1st step: create a max height texture with no image data ptr provided, so we can just check that padding fits
        auto pack_ranges = std::vector<stbtt_pack_range>(FontAtlas::atlas_size_count);
        for (i32 size_idx = 0; size_idx< FontAtlas::atlas_size_count; size_idx++){
            pack_ranges[size_idx] = stbtt_pack_range{
                .font_size = FontAtlas::atlas_min_height  + FontAtlas::atlas_step * size_idx,
                .first_unicode_codepoint_in_range = FontAtlas::code_point_begin_idx,
                .num_chars = FontAtlas::code_point_count,
                .chardata_for_range = packed_quads_per_size.at(size_idx).data(),
            };
        }
        auto const atlas_width_px = i32{2048};
        auto const max_tex_height_px = cast<i32>(m_vkPhysicalDevice.getProperties().limits.maxImageDimension2D);
        auto packing_rects = std::vector<stbrp_rect>(FontAtlas::code_point_count * FontAtlas::atlas_size_count);

        auto spc = stbtt_pack_context{};
        stbtt_PackBegin(
            &spc,
            nullptr, // We dont actually want to write any image data yet, we are simply querying the size required
            atlas_width_px,
            max_tex_height_px,
            0,
            FontAtlas::padding,
            nullptr
        );
        auto num_rects_packed = stbtt_PackFontRangesGatherRects(
            &spc,
            &font_info,
            pack_ranges.data(),
            pack_ranges.size(),
            packing_rects.data()
        );
        stbtt_PackFontRangesPackRects(
            &spc,
            packing_rects.data(),
            packing_rects.size()
        );
        stbtt_PackEnd(&spc);
        // now we have all of the packing rects
        //
        auto atlas_height_px = 0;
        for (auto const& rect: packing_rects | std::views::take(num_rects_packed)){
            ASSERT(rect.was_packed, "Glyphs exceed max texture height of device, some remain unpacked.",max_tex_height_px);

            auto const rect_topmost_y = rect.y + rect.h;
            atlas_height_px = std::max(atlas_height_px, rect_topmost_y);
        }
        atlas_height_px += FontAtlas::padding; 

        // 2nd step: With the height checked and limited as much as possible, we now do the actual packing:
        auto font_bitmap_pixels_raw = std::vector<std::byte>(atlas_width_px * atlas_height_px);
        auto font_bitmap_pixels = ByteSpan{font_bitmap_pixels_raw};
        stbtt_PackBegin(
            &spc,
            font_bitmap_pixels.get_ptr_as<uchar>(),
            atlas_width_px,
            atlas_height_px,
            0,
            FontAtlas::padding,
            nullptr
        );
        auto pack_okay = stbtt_PackFontRanges(
            &spc,
            font_file_data.get_ptr_as<uchar const>(),
            0,
            pack_ranges.data(),
            pack_ranges.size()
        );
        stbtt_PackEnd(&spc);
        ASSERT(
            pack_okay, 
            "Failure in packing of font data despite sufficient space in texture",
            atlas_width_px,
            atlas_height_px
        );

        stbi_write_png(
            "test_bitmap.png",
            atlas_width_px,
            atlas_height_px,
            1,
            font_bitmap_pixels.data(),
            atlas_width_px
        );
        auto atlas_image_data = ImageData::from_buffer(
            font_bitmap_pixels.span(),
            ImageData::Metadata{
                .file_name = std::string("(FontAtlas) ") + std::string(file_path),
                .px_w = atlas_width_px,
                .px_h = atlas_height_px,
                .n_channels = 1,
                .vk_format = FontAtlas::img_format,
            }
        );
        return FontAtlas{
            .file_contents = std::move(file_contents),
            .font_info = font_info,
            .packed_quads_per_size = packed_quads_per_size,
            .atlas_texture = make_texture_2d(
                atlas_image_data
            ),
            .atlas_px_width = atlas_width_px,
            .atlas_px_height = atlas_height_px,

        };
    }

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

