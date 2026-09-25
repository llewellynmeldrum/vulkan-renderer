#pragma once 
#include "types.hpp"
#include "vk_types.hpp"
#include "vk_format_traits.hpp"
struct ImageData{
    //
    struct Metadata{
        static constexpr i32 bytes_per_channel{1};
        std::string file_name = "n/a";
        i32 px_w{};
        i32 px_h{};
        i32 n_channels{};
        vk::Format vk_format{};
    };

    [[nodiscard]] static auto from_filename(
        std::string_view filename,
        bool load_alpha_channel=true
    ) -> ImageData;

    [[nodiscard]] static auto from_buffer(
        std::span<const std::byte> raw_data,
        ImageData::Metadata const& meta
    ) -> ImageData;


    static constexpr auto image_size_bytes(ImageData::Metadata const& meta) -> u32{
        auto const n_pixels = meta.px_w * meta.px_h;
        auto const bytes_per_pixel = meta.n_channels * meta.bytes_per_channel ;
        return n_pixels * bytes_per_pixel;
    }

    auto image_size_bytes()       const -> u32;
    auto get_extent2d()     const -> vk::Extent2D;


    // Either filled in by stbi_image_load in from_filename, or by the user in from_buffer.
    Metadata meta;
    std::vector<std::byte> buf;
};
