#include "renderer2d.hpp"
#include "cpu_mesh.hpp"

#include "format_specs.hpp"
auto Renderer2D::clear() -> void{
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
    add_quad(QuadVertexPositions2D{
        bl,
        br,
        tr,
        tl,
    });
}
auto Renderer2D::add_circle( glm::vec2 wCentre, f32 wRadius) -> void{
    ASSERT(false, "UNIMPLEMENTED");
}
auto Renderer2D::add_text(glm::vec2 wTopLeft, glm::vec2 wExtents) -> void{
    ASSERT(false, "UNIMPLEMENTED");
}


auto Renderer2D::add_quad(QuadVertexPositions2D positions) -> void {
    QuadVertices2D out{};
    auto const& draw_state = get_draw_state();
    for (int i = 0; i<positions.size(); i++){
        out[i].pos = positions[i];
        out[i].color = draw_state.fill_color;
        out[i].texCoord = vtx_raw_data::ccw_quad_verts[i].texCoord;
        auto const width  = positions.m_bot_right.x - positions.m_bot_left.x;
        auto const height = positions.m_top_left.y - positions.m_bot_right.y;
        auto const center = positions.m_bot_left + glm::vec2{width,height}*0.5f;
        LOG_DBG("tl:{}, tr:{}, bl:{}, br:{}, center:{}",positions.m_top_left, positions.m_top_right,positions.m_bot_left,positions.m_bot_right, center);
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

