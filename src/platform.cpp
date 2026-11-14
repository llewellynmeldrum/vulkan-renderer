#include "platform.hpp"
#include "stable_type_name.hpp"
Platform::Platform(
    glm::vec2 desiredLogicalSize,
    std::string_view window_name
) {
    LOG_DBG("Initializing {} ", stable_type_name<THIS_TYPE>());
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

    toggle_relative_cursor_mode();
    m_pixelDensity = SDL_GetWindowPixelDensity(m_window);
    if (m_pixelDensity == 0.0f){
        LOG_FATAL("Failed to get window pixel density: {}", SDL_GetError());
    }
    update_window_size(get_window_extent());
}

auto Platform::toggle_relative_cursor_mode() -> void{
    relative_cursor_mode = !relative_cursor_mode;
    if (!relative_cursor_mode){
        SDL_WarpMouseInWindow(m_window,m_windowExtent.x*0.5f, m_windowExtent.y*0.5f);
    }
    if (!SDL_SetWindowRelativeMouseMode(m_window, relative_cursor_mode)){
        LOG_FATAL("Failed to set relative mouse mode: {}", SDL_GetError());
    }
}

auto Platform::get_window_handle() const -> SDL_Window* {
    ASSERT(m_window != nullptr, "Window must be initialized before grabbing handle.");
    return m_window;
}

[[nodiscard]]
auto Platform::get_window_pixel_size() const -> glm::vec2 {
    int w{},h{};
    SDL_GetWindowSizeInPixels(m_window, &w,&h);
    return glm::vec2{w,h};
}

[[nodiscard]]
auto Platform::get_window_extent_pixels() const -> glm::vec2 {
    i32 w{},h{};
    if (!SDL_GetWindowSizeInPixels(m_window, &w,&h)){
        LOG_FATAL("Unable to get window size from sdl,{}", SDL_GetError());
    }
    return glm::vec2{w,h};
}

[[nodiscard]] 
auto Platform::get_window_extent() const -> glm::vec2 {
    return pixel_to_logical(get_window_extent_pixels());
}

auto Platform::capture_cursor() -> void{
    SDL_SetWindowRelativeMouseMode(m_window, true);
    m_cursor_is_captured = true;
}
auto Platform::uncapture_cursor() -> void{
    SDL_SetWindowRelativeMouseMode(m_window, false);
    m_cursor_is_captured = false;
}
auto Platform::update_window_size(glm::vec2 windowExtent) -> void {
    m_windowExtent = {windowExtent.x,windowExtent.y};
    m_windowPixelExtent = logical_to_pixel(windowExtent);
}
auto Platform::cleanup() -> void{
    SDL_DestroyWindow(m_window);
}
