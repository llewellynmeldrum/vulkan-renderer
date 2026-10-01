#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
#include "glm/ext/vector_int3.hpp"
#include <libassert/assert.hpp>
template<typename T, glm::ivec3 tExtents>
struct Array3D{
    static constexpr auto m_extents = tExtents;
    static constexpr auto m_xExtent = m_extents.x;
    static constexpr auto m_yExtent = m_extents.y;
    static constexpr auto m_zExtent = m_extents.z;
    static constexpr auto m_size = m_extents.x * m_extents.y * m_extents.z;

    constexpr Array3D(T const& initial_val) noexcept
        :m_data(std::vector<T>(m_size, initial_val))
    {};

    constexpr Array3D() noexcept
        :m_data(std::vector<T>(m_size, T{}))
    {};

    std::vector<T> m_data;
    decltype(auto)  get_mdspan(this auto& self) noexcept{
        return std::mdspan(self.m_data.data(), m_xExtent , m_yExtent , m_zExtent);
    }

    static constexpr auto size() noexcept{
        return m_size;
    }

    decltype(auto) operator[](this auto& self, std::size_t x, std::size_t y, std::size_t z) noexcept{
        return self.get_mdspan()[x,y,z];
    }

    decltype(auto) at(this auto& self, std::size_t x, std::size_t y, std::size_t z){
        ASSERT(x < m_xExtent && x >= 0);
        ASSERT(y < m_yExtent && y >= 0);
        ASSERT(z < m_zExtent && z >= 0);
        return self.get_mdspan()[x,y,z];
    }

    constexpr auto begin(this auto& self) noexcept{
        return self.m_data.begin();
    }
    constexpr auto end(this auto& self) noexcept{
        return self.m_data.end();
    }
};
