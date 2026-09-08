#pragma once 
#include "cpu_mesh.hpp"
#include "heightmap.hpp"
struct World{
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

    }
};
