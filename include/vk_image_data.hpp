#pragma once 
#include "types.hpp"
#include "vk_types.hpp"
struct ImageData{
    static constexpr i32 img_bytes_per_channel {1}; 
    static constexpr i32 desired_channels {4}; 
    static constexpr auto vk_format = vk::Format::eR8G8B8A8Srgb;

    auto free_buffer()      -> void;
    auto size_bytes()       -> u32;
    auto get_extent2d()     -> vk::Extent2D;

    static auto load_from_filename(std::string filename) -> ImageData;

    i32 px_w{}, px_h{}, n_src_channels{};
    std::span<std::byte> span;
};
