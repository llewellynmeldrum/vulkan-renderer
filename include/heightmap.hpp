#pragma once 

#include <mdspan>

#include "FastNoiseLite.h"
#include "glm_types.hpp"
#include "minmax.hpp"
#include "types.hpp"
#include "cpu_mesh.hpp"

struct HeightmapCreateInfo{
    glm::vec3 world_center;
    f32 extentX;
    f32 extentZ;
    f32 noise_freq;
    MinMax<f32> boundsY{-1,+1};
};

using NoiseType = FastNoiseLite::NoiseType;
using FractalType = FastNoiseLite::FractalType;
struct Heightmap{
public:
    Heightmap()=default;
    Heightmap( HeightmapCreateInfo info)
        : m_world_center(info.world_center)
        , m_extentX(info.extentX)
        , m_extentZ(info.extentZ)
        , m_boundsY(info.boundsY)
        , m_noiseGen(
            FastNoiseLite(m_seed)
                .SetNoiseType(NoiseType::OpenSimplex2)
                .SetFrequency(info.noise_freq)
        )
    {}

    f32 sample_height(f32 x, f32 z) const noexcept{
        auto const noise_sample = m_noiseGen.GetNoise(x,z);
        auto const noise01 = std::clamp((noise_sample + 1.0f) / 2.0f ,0.0f,1.0f);
        return m_boundsY.map01(noise01);
    }

    glm::vec3 m_world_center{0.0f,0.0f,0.0f};
    f32 m_extentX{0.0f};
    f32 m_extentZ{0.0f};
    MinMax<f32> m_boundsY{-1,+1};

private:
    FastNoiseLite m_noiseGen{};
    static constexpr int m_seed = 1338;
};

// in order to mesh a heightmap, you need to know how many samples there are going to be per horizontal axis;
struct HeightMapMeshCreateInfo {
    u32 num_x_samples;
    u32 num_z_samples;
};
CpuMesh mesh_heightmap(Heightmap const& heightmap, HeightMapMeshCreateInfo meshInfo);
