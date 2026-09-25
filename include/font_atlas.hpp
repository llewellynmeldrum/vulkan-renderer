#pragma once 

#include <libassert/assert.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <stb_image_write.hpp>
#include <stb_truetype.hpp>

#include "Texture2D.hpp"
#include "cppslop.hpp"
#include "types.hpp"
#include "file_io.hpp"
#include "vertex.hpp"
template<typename T, typename ...Args>
concept constructs = std::constructible_from<T, Args...>;
struct FontAtlas{
    struct AlignedQuad{
        glm::vec2 tl_px;
        glm::vec2 br_px;

        glm::vec2 tl_uv;
        glm::vec2 br_uv;
    };
    struct SizeSelection{
        f32 atlas_size{};
        f32 scale_correction_factor{1.0f};
        i32 atlas_idx; // smallest size = 0 
    };
    auto clear() -> void{
        atlas_texture.clear();
    }

    // Now, the function should advance the pen and return a quad.
    // It must now be called for each char in a requested mesh
    auto get_glyph_quad_2D(
        glm::vec2& pen,
        char ch,
        FontAtlas::SizeSelection const& selection
    ) -> FontAtlas::AlignedQuad;

    auto select_atlas_size(f32 true_font_size) -> FontAtlas::SizeSelection;

    std::vector<char> file_contents; // stbtt doesnt copy the font data, but instead has a non owning pointer. We must keep the file contents alive for the duration of the atlases lifetime.
    stbtt_fontinfo font_info;

    // Vector indexed by atlas_size_idx, containing packedchar's indexed by code_point. 
    // i.e packed_quads_vec_per_size[i][j] = the packedchar for size i and code point j.
    std::vector<std::vector<stbtt_packedchar>> packed_quads_per_size;
    Texture2D atlas_texture;
    i32 atlas_px_width{0};
    i32 atlas_px_height{0};



    static constexpr auto img_format = vk::Format::eR8Unorm;
    static constexpr auto padding = cast<u32>(1);
    static constexpr auto code_point_begin_idx = static_cast<u32>(' ');
    static constexpr auto code_point_end_idx = static_cast<u32>('~') + 1;
    static constexpr auto code_point_count = static_cast<u32>(code_point_end_idx - code_point_begin_idx);
    static constexpr auto atlas_min_height  = 8.0f;
    static constexpr auto atlas_step        = 4.0f;
    static constexpr auto atlas_size_count = static_cast<std::size_t>(30); // [8,128] is a solid range
    static constexpr auto atlas_max_height = atlas_min_height + atlas_step * atlas_size_count;
    static consteval std::array<f32,atlas_size_count> compute_atlas_sizes(){
        std::array<f32, atlas_size_count> res{};
        for (int i = 0; i<atlas_size_count; i++){
            res[i] = atlas_min_height + atlas_step * i;
        }
        return res;
    }
    std::array<f32,atlas_size_count> atlas_sizes = compute_atlas_sizes();
};
// Find the smallest atlas size which is >= than font_size
// Otherwise, pick the largest size
inline auto FontAtlas::get_glyph_quad_2D(
    glm::vec2& pen,
    char ch,
    FontAtlas::SizeSelection const& selection
) -> FontAtlas::AlignedQuad{
    auto char_index = ch - code_point_begin_idx;
    auto q = stbtt_aligned_quad{};
    stbtt_GetPackedQuad(
        packed_quads_per_size.at(selection.atlas_idx).data(),
        atlas_px_width,
        atlas_px_height, 
        char_index,
        &pen.x,
        &pen.y,
        &q,
        false
    );
    return AlignedQuad{
        .tl_px = glm::vec2{q.x0, q.y0 * selection.scale_correction_factor},
        .br_px = glm::vec2{q.x1, q.y1 * selection.scale_correction_factor},
        .tl_uv = glm::vec2{q.s0, q.t0},
        .br_uv = glm::vec2{q.s1, q.t1},
    };
        


}
// I now have the text quads rendering properly.
// All i need at this point is to apply the texture to the quads.
inline auto FontAtlas::select_atlas_size(f32 true_font_size) -> FontAtlas::SizeSelection{
    std::optional<f32> selection = std::nullopt;
    i32 selected_idx = FontAtlas::atlas_size_count-1; // fallback to max height
    for (i32 size_idx = 0; size_idx < FontAtlas::atlas_size_count; size_idx++){
        auto atlas_size = FontAtlas::atlas_sizes[size_idx];
        if (atlas_size >= true_font_size){
            selection = atlas_size;
            selected_idx = size_idx;
            break;
        }
    }
    // Fallback to the max size
    f32 selected_atlas_size = selection.value_or(atlas_sizes.back());
    f32 scale_correction_factor = selected_atlas_size / true_font_size;
    // The quad must be scaled by this factor in order to properly represent the requested font size
    LOG_DBG("Selected atlas_size:{} for size:{}. Correction: {}",
            selected_atlas_size, true_font_size, scale_correction_factor);
    return {
        .atlas_size = selected_atlas_size,
        .scale_correction_factor = scale_correction_factor,
        .atlas_idx= selected_idx,
    };

}
