#include "renderer2d.hpp"
#include "cpu_mesh.hpp"

#include "file_io.hpp"
#include "format_specs.hpp"
#include "cpp_slang_shared.hpp"
#include "stb_truetype.hpp"
#include "stb_image_write.hpp"
#include <span>


auto Renderer2D::init(
) -> void {

}

auto Renderer2D::clear() -> void {
    ASSERT(m_gpu_mesh.m_vertex_count > 0);
    ASSERT(m_gpu_mesh.m_index_count > 0);
//    m_cpu_mesh.clear();
    m_gpu_mesh.clear();
}

auto Renderer2D::add_square(glm::vec2 wTopLeft, f32 wExtent) 
-> void{
    add_rect(wTopLeft,{wExtent,wExtent});
}

auto Renderer2D::add_rect(glm::vec2 wTopLeft, glm::vec2 wExtents)
-> void{
    auto const width = wExtents.x;
    auto const height = wExtents.y;

    auto const tl = wTopLeft;

    auto const bl = tl + glm::vec2{0.0f,    +height};
    auto const br = tl + glm::vec2{width,   +height};
    auto const tr = tl + glm::vec2{width,   0.0f};
    add_quad(
        QuadVertexPositions2D{
            bl,
            br,
            tr,
            tl,
        },
        shapeID_Quad
    );
}
auto Renderer2D::add_circle( glm::vec2 wCentre, f32 wRadius) -> void{
    wRadius *= circle_edge_correction_scale;
    auto const width = wRadius * 2.0f;
    auto const height = wRadius * 2.0f;

    auto const wTopLeft = wCentre - glm::vec2{+wRadius,+wRadius};
    auto const tl = wTopLeft;
    auto const bl = tl + glm::vec2{0.0f,    +height};
    auto const br = tl + glm::vec2{width,   +height};
    auto const tr = tl + glm::vec2{width,   0.0f};
    add_quad(
        QuadVertexPositions2D{
            bl,
            br,
            tr,
            tl,
        },
        shapeID_Circle);
}
// Vertex data: (per ???)
// 
// Instance data: (per quad)
//      selected_atlas_idx      (eg. 0 = Arial_16pt)
//      selected_glyph_idx      (eg. 2 = Q)
//
//
//

// uploaded in an SSBO?
struct GlyphLayout{
    glm::vec2 origin;
    f32 glyph_width;
    f32 glyph_height;
};
//  GlyphLayout[N] (where N = number of glyphs) 
//
struct GlyphInstance{
    u32 glyph_layout_idx;
    glm::vec2 offset_pos;
};
/*
 let layout = glyphLayouts[instance.glyph_layout_idx];
 let pos = layout.origin + {
                 layout.col * layout.txt_width,
                 layout.row * layout.txt_height
            }
*/
// pos = 
auto Renderer2D::add_text(std::string_view text, glm::vec2 wTopLeft) -> void {
    // NOTE: No wrapping for now.

    auto const draw_st = get_draw_state();
    // 1. select the atlas 
    auto const font_size_selection = font_atlas.select_atlas_size(draw_st.font_height);

    auto pen = glm::vec2{wTopLeft};
    for (const auto ch: text){
        auto quad_corners = font_atlas.get_glyph_quad_2D(pen,ch,font_size_selection);
        add_glyph_quad(quad_corners, font_size_selection);
    }
}


auto Renderer2D::add_glyph_quad(
    FontAtlas::AlignedQuad const& quad,
    FontAtlas::SizeSelection const& size_selection
) -> void{
    auto const draw_st = get_draw_state();
    auto w_px = quad.br_px.x - quad.tl_px.x;
    auto h_px = quad.br_px.y - quad.tl_px.y;// NOTE: px is y down


    auto w_uv = quad.br_uv.x - quad.tl_uv.x;
    auto h_uv = quad.tl_uv.y - quad.br_uv.y;//NOTE: uv is y up

    auto const center_px = quad.tl_px + glm::vec2{w_px,h_px}*0.5f;
    auto res = Renderer2D::QuadVertices2D{
        // TOP LEFT
        Renderer2D::Vertex{
            .pos = quad.tl_px,
            .color = draw_st.fill_color,
            .uv_pos = quad.tl_uv,
            .shape_center_pos = center_px,
            .shapeID = shapeID_TextGlyph.get(),
        },
        // TOP RIGHT
        Renderer2D::Vertex{
            .pos = quad.tl_px + glm::vec2{w_px,0.0f},
            .color = draw_st.fill_color,
            .uv_pos = quad.tl_uv + glm::vec2{w_uv, 0.0f},
            .shape_center_pos = center_px,
            .shapeID = shapeID_TextGlyph.get(),
        },
        // BOT RIGHT
        Renderer2D::Vertex{
            .pos = quad.br_px,
            .color = draw_st.fill_color,
            .uv_pos = quad.br_uv,
            .shape_center_pos = center_px,
            .shapeID = shapeID_TextGlyph.get(),
        },
        // BOT LEFT
        Renderer2D::Vertex{
            .pos = quad.br_px - glm::vec2{w_px,0.0f},
            .color = draw_st.fill_color,
            .uv_pos = quad.br_uv - glm::vec2{w_uv, 0.0f},
            .shape_center_pos = center_px,
            .shapeID = shapeID_TextGlyph.get(),
        },
    };
    m_cpu_mesh.add_quad(res.span());
}
//auto Renderer2D::add_glyph_quad(glm::vec2 glyph_top_left, glm::vec2 uv_pos) -> void{
////    auto const glyph_width = stbtt_Width
 //   auto const tl = glyph_top_left;
 //   auto const bl = tl + glm::vec2{0.0f,    +height};
 //   auto const br = tl + glm::vec2{width,   +height};
 //   auto const tr = tl + glm::vec2{width,   0.0f};
 //   add_quad(
 //       QuadVertexPositions2D{
 //           bl,
 //           br,
 //           tr,
 //           tl,
 //       },
 //   QuadVertices2D out{};
 //   auto const& draw_state = get_draw_state();
 //   for (int i = 0; i<positions.size(); i++){
 //       out[i].pos = positions[i];
 //       out[i].color = draw_state.fill_color;
 //       out[i].uv_pos = vtx_raw_data::ccw_quad_verts[i].uv_pos;
 //       out[i].shapeID = shapeID.get();
 //       auto const width  = positions.m_bot_right.x - positions.m_bot_left.x;
 //       auto const height = positions.m_top_left.y - positions.m_bot_right.y;
 //       auto const center = positions.m_bot_left + glm::vec2{width,height}*0.5f;
////        LOG_DBG("tl:{}, tr:{}, bl:{}, br:{}, center:{}",positions.m_top_left, positions.m_top_right,positions.m_bot_left,positions.m_bot_right, center);
 //       out[i].shape_center_pos = center;
 //   }
 //   m_cpu_mesh.add_quad(out.span());
// }
auto Renderer2D::add_quad(QuadVertexPositions2D positions, QuadUvPositions2D uv_positions, BitMask shapeID) -> void{
    QuadVertices2D out{};
    auto const& draw_state = get_draw_state();
    for (int i = 0; i<positions.size(); i++){
        out[i].pos = positions[i];
        out[i].color = draw_state.fill_color;
        out[i].uv_pos = vtx_raw_data::ccw_quad_verts[i].uv_pos;
        out[i].shapeID = shapeID.get();
        auto const width  = positions.m_bot_right.x - positions.m_bot_left.x;
        auto const height = positions.m_top_left.y - positions.m_bot_right.y;
        auto const center = positions.m_bot_left + glm::vec2{width,height}*0.5f;
//        LOG_DBG("tl:{}, tr:{}, bl:{}, br:{}, center:{}",positions.m_top_left, positions.m_top_right,positions.m_bot_left,positions.m_bot_right, center);
        out[i].shape_center_pos = center;
    }
    m_cpu_mesh.add_quad(out.span());
}
auto Renderer2D::add_quad(QuadVertexPositions2D positions, BitMask shapeID) -> void {
    QuadVertices2D out{};
    auto const& draw_state = get_draw_state();
    for (int i = 0; i<positions.size(); i++){
        out[i].pos = positions[i];
        out[i].color = draw_state.fill_color;
        out[i].uv_pos = vtx_raw_data::ccw_quad_verts[i].uv_pos;
        out[i].shapeID = shapeID.get();
        auto const width  = positions.m_bot_right.x - positions.m_bot_left.x;
        auto const height = positions.m_top_left.y - positions.m_bot_right.y;
        auto const center = positions.m_bot_left + glm::vec2{width,height}*0.5f;
//        LOG_DBG("tl:{}, tr:{}, bl:{}, br:{}, center:{}",positions.m_top_left, positions.m_top_right,positions.m_bot_left,positions.m_bot_right, center);
        out[i].shape_center_pos = center;
    }
    m_cpu_mesh.add_quad(out.span());
}

auto Renderer2D::set_draw_state(DrawState const& v) -> void {
    draw_state_stack.top() = v;
}
auto Renderer2D::push_draw_state(DrawState const& v) -> void{
    draw_state_stack.push(v);
}
auto Renderer2D::pop_draw_state(void) -> void{
    draw_state_stack.pop();
}

