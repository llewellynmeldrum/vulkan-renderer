#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
#include <span>
#include <cstddef>
struct ByteSpan{
    std::span<std::byte> m_data;

    template<typename AsType>
    auto get_ptr_as(this auto& self, std::size_t offset=0uz) -> AsType*{
        return std::bit_cast<AsType*>(self.m_data.data() + offset);
    }
    auto data(this auto& self) -> decltype(auto){
        return self.m_data.data();
    }
    auto span(this auto& self) -> decltype(auto){
        return self.m_data;
    }
};
