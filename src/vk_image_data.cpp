#include "vk_image_data.hpp"
#include "stb_image.hpp"
#include "logger.hpp"

void ImageData::free_buffer(){
    stbi_image_free(span.data());
}
u32 ImageData::size_bytes(){
    return img_bytes_per_channel * px_w * px_h * desired_channels;
}
vk::Extent2D ImageData::get_extent2d(){
    return vk::Extent2D{
        static_cast<u32>(px_w),
        static_cast<u32>(px_h),
    };
}
ImageData ImageData::load_from_filename(std::string filename){
    ImageData img{};
    auto* img_data = stbi_load(filename.c_str(), &img.px_w,&img.px_h,&img.n_src_channels,img.desired_channels);
    img.span = std::span(reinterpret_cast<std::byte*>(img_data), img.size_bytes());
    if (!img_data){
        LOG_FATAL(
            "Unable to load image at '{}'. reason:{}",
            filename,
            stbi_failure_reason()
        );
    }
    return img;
}
