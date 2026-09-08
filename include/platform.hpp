#pragma once 

#include "shared_transformations.hpp"
#include "types.hpp"
#include "glm_types.hpp"
#include "logger.hpp"
#include "sdl3_types.hpp"

struct Platform{
    auto init(
        glm::vec2 desiredLogicalSize,
        std::string_view window_name ="Test window"sv
    ) -> void;

    auto handle_window_resize(glm::vec2 windowLogicalExtent)
    -> void;
    auto cleanup() -> void;

    auto get_window_handle() const
    -> SDL_Window*;

    [[nodiscard]] 
    auto get_window_pixel_size() const 
    -> glm::vec2;

    [[nodiscard]]
    auto get_window_extent_pixels() const 
    -> glm::vec2;

    [[nodiscard]] 
    auto get_window_extent_logical() const 
    -> glm::vec2;


    SDL_Window*                 m_window{nullptr};
    glm::vec2                   m_windowLogicalExtent{};
    glm::vec2                   m_windowPixelExtent{};
    static constexpr glm::vec2  m_windowNdcExtent{2, 2};
    f32                         m_pixelSize{0.0f};

private:
    glm::vec2 logical_to_ndc(glm::vec2 pLogical) const{ return detail::logical_to_ndc(pLogical, m_windowLogicalExtent);}
    glm::vec2 ndc_to_logical(glm::vec2 pNdc) const{ return detail::ndc_to_logical(pNdc, m_windowLogicalExtent);}
    glm::vec2 logical_to_pixel(glm::vec2 pLogical) const{ return detail::logical_to_pixel(pLogical,m_pixelSize);}
    glm::vec2 pixel_to_logical(glm::vec2 pPixel) const{ return detail::pixel_to_logical(pPixel,m_pixelSize);}

};
