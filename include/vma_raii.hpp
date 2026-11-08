#pragma once 
#include "vk_types.hpp"
#include "libassert/assert.hpp"
template<typename T> 
std::string whois(const T& val){
    auto type_str = libassert::pretty_type_name<T>();
    if constexpr(std::is_pointer_v<T>){
        return std::format("{} ({})", type_str, static_cast<void const*>(val));
    }else if constexpr( std::formattable<T,char>){
        return std::format("{} {}", type_str, val);
    }else{
        return std::format("{} (unformattable)", type_str);
    }
}
struct RaiiVmaAllocator{
    RaiiVmaAllocator() 
        : m_is_initialized(false)
    {}

    RaiiVmaAllocator(nullptr_t) 
        : m_is_initialized(false)
    {}

    ~RaiiVmaAllocator() { 
        if (m_is_initialized){
            LOG_WARN("Warning!! Dtor called on uninitialized `{}`", whois(this));
        }
        vmaDestroyAllocator(m_raw_handle); 
    }

    static constexpr auto create(VmaAllocatorCreateInfo info)
    -> RaiiVmaAllocator{
        auto raw_handle = VmaAllocator{}; 
        auto res = static_cast<vk::Result>(vmaCreateAllocator(&info,&raw_handle));
        ASSERT(res == vk::Result::eSuccess);
        return RaiiVmaAllocator(raw_handle);
    }

    bool m_is_initialized = false;
    VmaAllocator m_raw_handle{nullptr};

    RaiiVmaAllocator(RaiiVmaAllocator && victim)
        :m_is_initialized(std::exchange(victim.m_is_initialized, false))
        ,m_raw_handle(std::exchange(victim.m_raw_handle, nullptr))
    {}
    RaiiVmaAllocator& operator=(RaiiVmaAllocator && victim){
        if (&victim != this){
            vmaDestroyAllocator(this->m_raw_handle);
            this->m_is_initialized = std::exchange(victim.m_is_initialized, false);
            this->m_raw_handle = std::exchange(victim.m_raw_handle, nullptr);
        }
        return *this;
    }
    RaiiVmaAllocator(const RaiiVmaAllocator &) = delete;
    RaiiVmaAllocator &operator=(const RaiiVmaAllocator &) = delete;


    VmaAllocator get(this auto& self) noexcept{
        return self.m_raw_handle;
    }
    VmaAllocator operator*(this auto& self) noexcept{
        return self.get();
    }
    operator VmaAllocator(this auto& self) noexcept{
        return self.get();
    }
private:
    RaiiVmaAllocator(VmaAllocator raw_handle) 
        : m_is_initialized(true)
        , m_raw_handle(raw_handle)
    {}

};
