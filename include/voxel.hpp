#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus

#include <vector>
#include <mdspan>

#include <libassert/assert.hpp>
#include <glm/ext/vector_float3.hpp>

#include <glm/ext/vector_int3_sized.hpp>
#include <glm/ext/vector_uint3_sized.hpp>
#include "primitive_types.hpp"
#include "array3d.hpp"
#include "shared/voxel_shared.hpp"

struct Voxel{
    VoxelMaterial m_material;
};

template<glm::ivec3 tExtents>
using VoxelGrid = Array3D<Voxel, tExtents>;

