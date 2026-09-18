
#include <print>
#include "heightmap.hpp"
#include "logger.hpp"
#include "minmax.hpp"

#include <FastNoiseLite.h>
#include "color_utils.hpp"
#include "vertex_raw_data.hpp"
static constexpr auto N_CORNERS{4};
CpuMesh3D mesh_heightmap(Heightmap const& heightmap, HeightMapMeshCreateInfo meshInfo){
    auto mesh = CpuMesh3D{};
    // Start with a quad which simply samples at each corner point 
    
    auto color_from_y = [&heightmap](f32 y){
        auto val = heightmap.m_boundsY.unlerp(y);
        return glm::vec3{val,val,val};
    };
    auto const hextentX = heightmap.m_extentX * 0.5f;
    auto const hextentZ = heightmap.m_extentZ * 0.5f;
    auto const cx = 0;
    auto const cz = 0;

    // if you split a line in the middle, you have 2 line segments (subquads) for 3 'samples'
    auto x_subquad_count = meshInfo.num_x_samples - 1;
    auto z_subquad_count = meshInfo.num_z_samples - 1;

    auto x_subquad_extent =  heightmap.m_extentX / x_subquad_count;
    auto z_subquad_extent =  heightmap.m_extentZ / x_subquad_count; 
    auto const ox = cx - hextentX;
    auto const oz = cz - hextentZ;

//    LOG_DBG_EXPR(x_subquad_count);
//    LOG_DBG_EXPR(z_subquad_count);
//    LOG_DBG_EXPR(x_subquad_extent);
//    LOG_DBG_EXPR(z_subquad_extent);
    // subquad 0,0: 
    // s0 is at c0
    for (i32 ix = 0; ix<x_subquad_count; ix++){
        for (i32 iz = 0; iz<z_subquad_count; iz++){
            auto const x0 = ox + x_subquad_extent * ix;
            auto const x1 = ox + x_subquad_extent * (ix+1);
            auto const z0 = oz + z_subquad_extent * iz;
            auto const z1 = oz + z_subquad_extent * (iz+1);
            auto const corners_xz = std::array{
                vec2xz{x0, z0},
                vec2xz{x1, z0},
                vec2xz{x1, z1},
                vec2xz{x0, z1},
            };

            auto out_vertices  = std::array<Vertex3D,4>{};
            for (i32 i = 0; i<N_CORNERS ; i++){
                auto const x = corners_xz[i].x;
                auto const z = corners_xz[i].z;
                auto const y = heightmap.sample_height(x,z);
                out_vertices[i].pos = {x,y,z};
                out_vertices[i].color = color_from_y(y);
                out_vertices[i].uv_pos = vtx_raw_data::ccw_quad_verts[i].uv_pos;
            }
            mesh.add_quad(out_vertices);
        }
    }

    // This quad will sit at the bottom of the heightmap to indicate size.

    auto const x0 = cx - hextentX;
    auto const x1 = cx + hextentX;
    auto const z0 = cz - hextentZ;
    auto const z1 = cz + hextentZ;

    auto corners_xz = std::array{
        vec2xz{x0, z0},
        vec2xz{x1, z0},
        vec2xz{x1, z1},
        vec2xz{x0, z1},
    };
    auto out_vertices  = std::array<Vertex3D,4>{};
    for (i32 i = 0; i<N_CORNERS ; i++){
        auto const x = corners_xz[i].x;
        auto const z = corners_xz[i].z;
        auto const y = -heightmap.m_boundsY.range()/2.0f;
        out_vertices[i].pos = {x,y,z};
        out_vertices[i].color = make_rgb(255,255,255);
        out_vertices[i].uv_pos = vtx_raw_data::ccw_quad_verts[i].uv_pos;
    }
    mesh.add_quad(out_vertices);
    return mesh;
}
