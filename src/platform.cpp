#include "platform.hpp"
auto
Platform::init(
    glm::vec2 desiredLogicalSize,
    std::string_view window_name
)
-> void
{
    static constexpr auto init_flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
    static constexpr auto window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE; 
    // Consider using 
    // SDL_WINDOW_HIGH_PIXEL_DENSITY ;


    if (!SDL_Init(init_flags)){
        LOG_ERROR("Failed to init SDL3: {}", SDL_GetError());
    }
    m_window = SDL_CreateWindow(
        std::string(window_name).c_str(),
        desiredLogicalSize.x,
        desiredLogicalSize.y,
        window_flags
    );
    if (!m_window) {
        LOG_FATAL("Failed to init window : {}", SDL_GetError());
    }

    if (!SDL_SetWindowRelativeMouseMode(m_window, true)){
        LOG_FATAL("Failed to set relative mouse mode: {}", SDL_GetError());
    }
    m_pixelSize = SDL_GetWindowPixelDensity(m_window);
    if (m_pixelSize == 0.0f){
        LOG_FATAL("Failed to get window pixel density: {}", SDL_GetError());
    }
    handle_window_resize(get_window_extent_logical());
}

auto Platform::get_window_handle() const
-> SDL_Window*
{
    ASSERT(m_window != nullptr, "Window must be initialized before grabbing handle.");
    return m_window;
}

[[nodiscard]]
auto Platform::get_window_pixel_size() const 
-> glm::vec2
{
    int w{},h{};
    SDL_GetWindowSizeInPixels(m_window, &w,&h);
    return glm::vec2{w,h};
}

[[nodiscard]]
auto Platform::get_window_extent_pixels() const 
-> glm::vec2
{
    i32 w{},h{};
    if (!SDL_GetWindowSizeInPixels(m_window, &w,&h)){
        LOG_FATAL("Unable to get window size from sdl,{}", SDL_GetError());
    }
    return glm::vec2{w,h};
}

[[nodiscard]] 
auto Platform::get_window_extent_logical() const 
-> glm::vec2 
{
    return pixel_to_logical(get_window_extent_pixels());
}

auto Platform::handle_window_resize(glm::vec2 windowLogicalExtent)
-> void
{
    m_windowLogicalExtent = {windowLogicalExtent.x,windowLogicalExtent.y};
    m_windowPixelExtent = logical_to_pixel(windowLogicalExtent);
}
auto Platform::cleanup() -> void{
    SDL_DestroyWindow(m_window);
}
