#pragma once 
#include "vk_types.hpp"

template<vk::Format tFormat>
struct FormatTraits;

#define FMT_TRAIT(FORMAT, BYTES_PER_CHANNEL, N_CHANNELS)                \
template<>                                                              \
struct FormatTraits<FORMAT>{                                            \
    static constexpr std::size_t bytes_per_channel = BYTES_PER_CHANNEL; \
    static constexpr std::size_t n_channels = N_CHANNELS;               \
    static constexpr std::size_t pixel_size_bytes = N_CHANNELS * BYTES_PER_CHANNEL;               \
};

//                                      B/Channel          Num channels
FMT_TRAIT(vk::Format::eR8G8B8Srgb,      1uz,                3uz)
FMT_TRAIT(vk::Format::eR8G8B8A8Srgb,    1uz,                4uz)
