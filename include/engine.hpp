#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
#include <thread>

#include "platform.hpp"
#include "renderer.hpp"
#include "input.hpp"
#include "world.hpp"
#include "camera.hpp"

#include "transformations.hpp"
#include "mesh_id.hpp"

struct Engine{
 public:
    auto init() -> void;
    auto run() -> void;
    auto cleanup() -> void;

    Platform platform;
    Renderer rend;
    Input input;
    World world;
    Camera cam;
    auto ui_draw() -> void;

    bool m_shouldRender         {true};
    bool m_windowResized        {false};
    bool m_worldUpdatesPaused   {false};
    bool m_shouldQuit           {false};
    bool m_uiNavigationMode     {false};

    static inline Engine* instance = nullptr;
    static auto get_instance() -> Engine*;

    auto upload_heightmap(Heightmap const& heightmap)
    -> void;

 private:
    void toggle_ui_mode();
    void poll_events();
    void handle_window_resize();
    void handle_input_actions();
    void handle_scroll_motion(SDL_MouseWheelEvent const& wheel);
    void handle_mouse_motion(SDL_MouseMotionEvent const& wheel);
    void handle_event_reporting(SDL_Event const& ev);
    void report_quit_event();
    void report_minimize_event();
    void report_unminimize_event();
    void report_resize_event();
};
