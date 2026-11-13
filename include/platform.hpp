#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus

#include "transformations.hpp"
#include "primitive_types.hpp"
#include "glm_types.hpp"
#include "logger.hpp"
#include "sdl3_types.hpp"

struct Platform{
    Platform(
        glm::vec2 desiredLogicalSize,
        std::string_view window_name ="Test window"sv
    );

    auto capture_cursor() -> void;
    auto uncapture_cursor() -> void;
    auto update_window_size(glm::vec2 windowExtent) -> void;
    auto cleanup() -> void;

    bool relative_cursor_mode = false;

    auto toggle_relative_cursor_mode() -> void;
    auto get_window_handle() const -> SDL_Window*;

    [[nodiscard]] auto get_window_pixel_size() const -> glm::vec2;

    [[nodiscard]] auto get_window_extent_pixels() const -> glm::vec2;

    [[nodiscard]] auto get_window_extent() const -> glm::vec2;

    SDL_Window*                 m_window{nullptr};
    glm::vec2                   m_windowExtent{}; // unless otherwise stated, screen extents are assumed to be logical
    glm::vec2                   m_windowPixelExtent{};
    static constexpr glm::vec2  m_windowNdcExtent{2, 2};
    f32                         m_pixelDensity{0.0f};
    bool m_cursor_is_captured{false};

private:
    glm::vec2 logical_to_ndc(glm::vec2 pLogical) const{ return detail::logical_to_ndc(pLogical, m_windowExtent);}
    glm::vec2 ndc_to_logical(glm::vec2 pNdc) const{ return detail::ndc_to_logical(pNdc, m_windowExtent);}
    glm::vec2 logical_to_pixel(glm::vec2 pLogical) const{ return detail::logical_to_pixel(pLogical,m_pixelDensity);}
    glm::vec2 pixel_to_logical(glm::vec2 pPixel) const{ return detail::pixel_to_logical(pPixel,m_pixelDensity);}

};
