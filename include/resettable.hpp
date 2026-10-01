#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
template<typename T>
struct Resettable{
    Resettable(T const& from)
        :m_initial(from)
        ,m_val(from)
    {}
    T m_initial;
    T m_val;
    T& val(this auto& self) noexcept{
        return self.m_val; 
    }
    void reset() noexcept{
        m_val = m_initial;
    }
};
