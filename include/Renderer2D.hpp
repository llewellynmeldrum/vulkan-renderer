#pragma once 
#include <ranges>
#include <stack>
#include <ranges>
#include <span>

#include "cppslop.hpp"
#include "cpu_mesh.hpp"
#include "gpu_mesh.hpp"
#include "types.hpp"
#include "vk_types.hpp"
#include "common_concepts.hpp"
#include "glm_types.hpp"
#include "renderer2d_draw_state.hpp"
#include "vertex_raw_data.hpp"
#include "quad_vertices.hpp"
#include "cppslop.hpp"

struct Renderer2D{
public: // SECTION: SPECIAL MEMBER FNS ================================================================================

public: // SECTION: TYPES/TYPEDEFS ====================================================================================

    using Index = u32;
    using Vertex = Vertex2D;
    using QuadVertices2D = QuadVertices<Renderer2D::Vertex>;
    using QuadVertexPositions2D = QuadVertices<glm::vec2>;


public: // SECTION: PUBLIC FUNCTIONS ==================================================================================
    
    auto clear() -> void;
    auto add_rect(glm::vec2 wTopLeft, glm::vec2 wExtents) -> void;
    auto add_square(glm::vec2 wTopLeft, f32 wExtent) -> void;
    auto add_circle( glm::vec2 wCentre, f32 wRadius) -> void;
    auto add_text( glm::vec2 wTopLeft, glm::vec2 wExtents) -> void;

    auto get_draw_state(this auto& self) -> decltype(auto);
    auto set_draw_state(DrawState const& v) -> void ;
    auto push_draw_state(DrawState const& v) -> void;
    auto pop_draw_state(void) -> void;


public: // SECTION: PUBLIC MEMBERS ====================================================================================

    static constexpr auto IndicesPerQuad{6};
    static constexpr auto VerticesPerQuad{4};

    CpuMesh2D m_cpu_mesh;
    GpuMesh2D m_gpu_mesh;

    std::size_t m_quad_count{0};


private: // SECTION: PRIVATE FUNCTIONS ================================================================================

    // wrapper for m_cpu_mesh.add_quad();
    auto add_quad(QuadVertexPositions2D positions) -> void;


private: // SECTION: PRIVATE MEMBERS ========================================================================

    std::stack<DrawState> draw_state_stack = init_stack({DrawState{}});


};


// Template shit
inline auto Renderer2D::get_draw_state(this auto& self) -> decltype(auto) {
    return self.draw_state_stack.top();
}
