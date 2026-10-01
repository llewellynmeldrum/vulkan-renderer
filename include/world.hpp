#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
#include "cpu_mesh.hpp"
#include "heightmap.hpp"
#include "voxel.hpp"
struct World{
    static constexpr glm::ivec3 m_worldExtents{100, 100, 100};
    VoxelGrid<m_worldExtents> grid{};
    glm::vec3 center;
    //std::vector<CpuMesh> cpu_meshes;

    Heightmap m_heightmap{};
    static constexpr glm::vec2 m_heightmapExtents{100.0f,100.0f};
    void init(){
        m_heightmap = Heightmap(
            HeightmapCreateInfo{
                .world_center = glm::vec3{2.0f,2.0f, 4.0f},
                .extentX = m_heightmapExtents[0],
                .extentZ = m_heightmapExtents[1],
                .noise_freq = 0.1f,
                .boundsY = {-1,+1}
            }
        );
    }


    void per_frame_update(){
        // fill the voxel array with the heightmap data
//        for ()

    }
};
