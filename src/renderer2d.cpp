#include "renderer2d.hpp"
#include "cpu_mesh.hpp"

auto Renderer2D::make_cpu_mesh() const -> CpuMesh2D{
    return CpuMesh2D{m_vertices,m_indices};
}
auto Renderer2D::clear() -> void{
    ASSERT(m_gpu_mesh.m_vertex_count > 0);
    ASSERT(m_gpu_mesh.m_index_count > 0);
    m_gpu_mesh.clear();
}
auto Renderer2D::add_rect( glm::vec2 wTopLeft, glm::vec2 wExtents, f32 outlineThickness_px)
-> void{
    auto const width = wExtents.x;
    auto const height = wExtents.y;

    auto const tl = wTopLeft;

    auto const bl = tl + glm::vec2{0.0f,    -height};
    auto const br = tl + glm::vec2{width,   -height};
    auto const tr = tl + glm::vec2{width,   0.0f};
    add_quad(QuadVertexPositions2D{
        bl,
        br,
        tr,
        tl,
    });
}

auto Renderer2D::get_draw_state(this auto& self) -> decltype(auto) {
    return self.draw_state_stack.top();
}

auto Renderer2D::add_quad(QuadVertexPositions2D positions) -> void {
    auto const& draw_state = get_draw_state();
    auto const& index_offset = m_vertices.size();

    for (int i = 0; i<Renderer2D::VerticesPerQuad; i++){
        m_vertices.emplace_back(
            positions[i],
            draw_state.fill_color,
            vtx_raw_data::ccw_quad_verts[i].texCoord
        );
    }

    for (int i = 0; i<Renderer2D::IndicesPerQuad; i++){
        auto index = index_offset + vtx_raw_data::ccw_quad_indices[i];
        ASSERT(index < m_vertices.size());
        m_indices.emplace_back(index);
    }
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

