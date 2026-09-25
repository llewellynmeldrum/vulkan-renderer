#include "vk_image_data.hpp"
#include "stb_image.hpp"
#include "logger.hpp"
#include "range/v3/all.hpp"
#include "vulkan/vulkan_format_traits.hpp"

u32 ImageData::image_size_bytes() const{
    auto const calculated_size = ImageData::image_size_bytes(meta);
    auto const span_size = buf.size();
    ASSERT(calculated_size == span_size);
    return span_size;
}
vk::Extent2D ImageData::get_extent2d() const{
    return vk::Extent2D{
        static_cast<u32>(meta.px_w),
        static_cast<u32>(meta.px_h),
    };
}

auto ImageData::from_buffer(
    std::span<const std::byte> raw_data,
    ImageData::Metadata const& meta
) -> ImageData{
    return ImageData {
        .meta = meta,
        .buf = ranges::to<std::vector<std::byte>>(raw_data),
    };
}
ImageData ImageData::from_filename(std::string_view filename, bool load_alpha_channel){
    auto const desired_channels = 3 + load_alpha_channel;
    auto meta = ImageData::Metadata{
        .file_name = std::string(filename),
        // stbi should convert whatever format we have on disk into our desired channel count.
        // Eg if the file is rgba, and we request rgb, it will skip every fourth byte.
        // alternatively, if the file is rgb and we request RGBA, it will fill in the alpha with 255.
        // If we instead set n_channels to whatevers on file, that makes less sense.
        .n_channels = desired_channels,
        .vk_format = load_alpha_channel ? vk::Format::eR8G8B8A8Srgb : vk::Format::eR8G8B8Srgb,
    };

    auto n_channels_in_file = i32{};
    auto* img_data = stbi_load(
        std::string(filename).c_str(), 
        &meta.px_w,
        &meta.px_h,
        &n_channels_in_file,
        desired_channels
    );
    if (!img_data){
        LOG_FATAL(
            "Unable to load image at '{}'. reason:{}",
            filename,
            stbi_failure_reason()
        );
    }
            
    auto n_bytes = ImageData::image_size_bytes(meta);
    auto span = std::span(reinterpret_cast<std::byte*>(img_data),n_bytes);
    auto res = ImageData {
        .meta = meta,
        .buf = ranges::to<std::vector>(span),
    };
    stbi_image_free(img_data); // ranges::to<std::vector> created a copy belonging to the object
    return res;
}
