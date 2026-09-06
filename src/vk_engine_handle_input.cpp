
#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_oldnames.h"
#include "vk_engine.hpp"
#include "format_specs.hpp"
#include "vk_engine_input_keys.hpp"

void VkEngine::process_inputs(SDL_KeyboardEvent const& key_ev){
    static constexpr auto rotate_speed = f32{1.0f};
    static constexpr auto move_speed = f32{0.1f};

    // bug with key state
    if (just_pressed(KeyCode::T)){
            if (m_vkPolygonMode == vk::PolygonMode::eFill){
                m_vkPolygonMode = vk::PolygonMode::eLine;
            }
            else {
                m_vkPolygonMode = vk::PolygonMode::eFill;
            }
    }
    if (is_down(KeyCode::LEFT)) { m_cam.rotate_left(rotate_speed);  }
    if (is_down(KeyCode::RIGHT)) { m_cam.rotate_right(rotate_speed); }
    if (is_down(KeyCode::UP)) { m_cam.rotate_up(rotate_speed);    }
    if (is_down(KeyCode::DOWN)) { m_cam.rotate_down(rotate_speed);  }

    if (is_down(KeyCode::A)) { m_cam.move_left(move_speed);      }
    if (is_down(KeyCode::D)) { m_cam.move_right(move_speed);     }
    if (is_down(KeyCode::W)) { m_cam.move_forward(move_speed);   }
    if (is_down(KeyCode::S)) { m_cam.move_backward(move_speed);  }
    if (is_down(KeyCode::E) || is_down(KeyCode::SPACE)) { m_cam.move_up(move_speed);        }
    if (is_down(KeyCode::Q) || is_down(KeyCode::RCTRL)) { m_cam.move_down(move_speed);      }

    std::ranges::copy(keystate, keys_pressed_last_frame.begin());
}

void VkEngine::handle_scroll_motion(SDL_MouseWheelEvent const& wheel){
    glm::vec2 scroll = {wheel.x,wheel.y};
    i32 scroll_direction = wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1  : 1; 
    scroll *= scroll_direction;
    m_cam.vfov = glm::clamp(m_cam.vfov - scroll.y * m_cam.zoom_sens, 10.0f, 100.0f);
    LOG_DBG("{}",m_cam.vfov);
}

void VkEngine::handle_mouse_motion(SDL_MouseMotionEvent const& ev){
    m_cam.rotate_pitch(ev.yrel / m_windowLogicalSize.height);
    m_cam.rotate_yaw(ev.xrel / m_windowLogicalSize.width);
}

void VkEngine::handle_inputs(){
    SDL_Event e{};
    while ((SDL_PollEvent(&e)) != 0) {
        if (e.type == SDL_EVENT_QUIT) {
            m_shouldStopRunning = true;
        }
        if (e.type == SDL_EVENT_WINDOW_MINIMIZED) {
            m_shouldStopRendering = true;
        }
        if (e.type == SDL_EVENT_WINDOW_RESTORED) {
            m_shouldStopRendering = false;
        }
        if (e.type == SDL_EVENT_WINDOW_RESIZED) {
            m_framebufferResized = true;
        }
        if (e.type == SDL_EVENT_KEY_DOWN){
            auto scancode = SDL_GetScancodeFromKey(e.key.key,nullptr);
            keystate[scancode] = true;
        }
        if (e.type == SDL_EVENT_KEY_UP){
            auto scancode = SDL_GetScancodeFromKey(e.key.key,nullptr);
            keystate[scancode] = false;
        }

        if (e.type == SDL_EVENT_MOUSE_MOTION){
            handle_mouse_motion(e.motion);
        }
        if (e.type == SDL_EVENT_MOUSE_WHEEL){
            handle_scroll_motion(e.wheel);
        }
    }
    process_inputs({});
}
