#include "heightmap.hpp"
#include "logger.hpp"
#include "minmax.hpp"

#include "FastNoiseLite.h"
#include <print>
static constexpr auto N_CORNERS{4};
CpuMesh mesh_heightmap(Heightmap const& heightmap, HeightMapMeshCreateInfo meshInfo){
    auto mesh = CpuMesh{};
    // Start with a quad which simply samples at each corner point 
    
    auto color_from_y = [&heightmap](f32 y){
        auto val = heightmap.m_boundsY.unlerp(y);
        return glm::vec3{val,val,val};
    };
    auto white_quad_colors = std::array<glm::vec3,4>{
        glm::vec3{1.0f,1.0f,1.0f},
        glm::vec3{1.0f,1.0f,1.0f},
        glm::vec3{1.0f,1.0f,1.0f},
        glm::vec3{1.0f,1.0f,1.0f},
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
            auto corners_xz = std::array{
                vec2xz{x0, z0},
                vec2xz{x1, z0},
                vec2xz{x1, z1},
                vec2xz{x0, z1},
            };
            auto corners_xyz = std::array<glm::vec3,4>{};
            auto colors = std::array<glm::vec3,4>{};
            for (i32 i = 0; i<N_CORNERS ; i++){
                auto const x = corners_xz[i].x;
                auto const z = corners_xz[i].z;
                auto const y = heightmap.sample_height(x,z);
                corners_xyz[i] = {x,y,z};
                colors[i] = color_from_y(y);
            }
            mesh.add_quad(corners_xyz,colors);
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
    auto flat_bottom = std::array<glm::vec3,4>{};
    for (i32 i = 0; i<N_CORNERS ; i++){
        auto const wx = corners_xz[i].x;
        auto const wz = corners_xz[i].z;
        auto const wy = -heightmap.m_boundsY.range()/2.0f;
        flat_bottom[i] = {wx,wy,wz};
    }
    mesh.add_quad(flat_bottom,white_quad_colors);
    return mesh;
}
