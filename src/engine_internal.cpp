
#include "engine.hpp"
#include "magic_enum.hpp"

void Engine::handle_window_resize(){
    using namespace std::chrono_literals;
    auto windowPixelExtent = platform.get_window_pixel_size();
    while (windowPixelExtent.x == 0 || windowPixelExtent.y == 0){
        // window is minimized, we should wait busywait and keep polling events 
        std::this_thread::sleep_for(10ms);
        poll_events();
        windowPixelExtent = platform.get_window_extent_pixels();
    }

    auto const windowLogicalExtent = detail::pixel_to_logical(windowPixelExtent, platform.m_pixelSize);
    input.handle_window_resize(windowLogicalExtent);
    platform.handle_window_resize(windowLogicalExtent);

    rend.handle_window_resize(windowLogicalExtent);
}

void Engine::handle_input_actions(){
    auto rotate_speed = f32{1.0f};
    auto move_speed = f32{0.1f};

    cam.handle_mouse_movement(input.mouse_pos_logical_movement() / 1000.0f);

    if (input.just_pressed(KeyCode::T)){
        rend.toggle_wireframe();
    }
    if (input.just_pressed(KeyCode::ESCAPE)){
        rend.toggle_wireframe();
    }
    if (input.modifier_enabled(KeyModBits::LSHIFT)){
        move_speed *= 1.5f;
    }

    if (input.key_down(KeyCode::LEFT)) { cam.rotate_left(rotate_speed);  }
    if (input.key_down(KeyCode::RIGHT)) { cam.rotate_right(rotate_speed); }
    if (input.key_down(KeyCode::UP)) { cam.rotate_up(rotate_speed);    }
    if (input.key_down(KeyCode::DOWN)) { cam.rotate_down(rotate_speed);  }

    if (input.key_down(KeyCode::A)) { cam.move_left(move_speed);      }
    if (input.key_down(KeyCode::D)) { cam.move_right(move_speed);     }
    if (input.key_down(KeyCode::W)) { cam.move_forward(move_speed);   }
    if (input.key_down(KeyCode::S)) { cam.move_backward(move_speed);  }
    if (input.any_keys_down(KeyCode::E, KeyCode::SPACE)) { cam.move_up(move_speed);        }
    if (input.any_keys_down(KeyCode::Q, KeyCode::LCTRL)) { cam.move_down(move_speed);      }

    input.end_frame();
}

void Engine::handle_scroll_motion(SDL_MouseWheelEvent const& wheel){
    glm::vec2 scroll = {wheel.x,wheel.y};
    i32 scroll_direction = wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1  : 1; 
    scroll *= scroll_direction;
    cam.vfov = glm::clamp(cam.vfov - scroll.y * cam.zoom_sens, 10.0f, 100.0f);
    LOG_DBG("{}",cam.vfov);
}

void Engine::handle_mouse_motion(SDL_MouseMotionEvent const& ev){
}
void Engine::report_quit_event(){
    m_shouldQuit = true;
}
void Engine::report_minimize_event(){
    m_shouldRender = false;
}
void Engine::report_unminimize_event(){
    m_shouldRender = true;
}
void Engine::report_resize_event(){
    m_windowResized = true;
    rend.m_requiresSwapchainRecreation = true;
}

void Engine::handle_event_reporting(SDL_Event const& e){
    auto event_type = static_cast<SDL_EventType>(e.type);
    switch (event_type){
        case SDL_EVENT_QUIT:             { report_quit_event();                 break; }
        case SDL_EVENT_WINDOW_MINIMIZED: { report_minimize_event();             break; }
        case SDL_EVENT_WINDOW_RESTORED:  { report_unminimize_event();           break; }
        case SDL_EVENT_WINDOW_RESIZED:   { report_resize_event();               break; }
        case SDL_EVENT_KEY_DOWN:         { input.report_keydown(e.key);         break; }
        case SDL_EVENT_KEY_UP:           { input.report_keyup(e.key);           break; }
        case SDL_EVENT_MOUSE_MOTION:     { input.report_mouse_moved(e.motion);  break; }
        case SDL_EVENT_MOUSE_WHEEL:      { input.report_scroll(e.wheel);        break; }
        default:{
            LOG_WARN("Unhandled event of type '{}'.", magic_enum::enum_name(event_type));
            break;
        }
    };
}

void Engine::poll_events(){
    SDL_Event e{};
    while ((SDL_PollEvent(&e)) != 0) {
        handle_event_reporting(e);
    }
    input.cur.key_mod_state = KeyMod(SDL_GetModState());
}
