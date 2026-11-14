#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
#include "cpu_mesh.hpp"
#include "heightmap.hpp"
#include "voxel.hpp"

struct World{
    World();
    ~World() = default;
};
