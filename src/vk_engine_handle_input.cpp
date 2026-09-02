#include <SDL3/SDL.h>

#include "vk_engine.hpp"
#include "format_specs.hpp"

void VkEngine::handle_key_down(SDL_KeyboardEvent const& key_ev){
    static constexpr auto rotate_speed = f32{1.0f};
    static constexpr auto move_speed = f32{0.1f};
    switch(key_ev.key){
        case SDLK_T:{
            if (m_vkPolygonMode == vk::PolygonMode::eFill){
                m_vkPolygonMode = vk::PolygonMode::eLine;
            }
            else {
                m_vkPolygonMode = vk::PolygonMode::eFill;
            }
        } break;

        case SDLK_LEFT: { m_cam.rotate_left(rotate_speed);  } break;
        case SDLK_RIGHT:{ m_cam.rotate_right(rotate_speed); } break;
        case SDLK_UP:   { m_cam.rotate_up(rotate_speed);    } break;
        case SDLK_DOWN: { m_cam.rotate_down(rotate_speed);  } break;

        case SDLK_A:{ m_cam.move_left(move_speed);      } break;
        case SDLK_D:{ m_cam.move_right(move_speed);     } break;
        case SDLK_W:{ m_cam.move_forward(move_speed);   } break;
        case SDLK_S:{ m_cam.move_backward(move_speed);  } break;
        case SDLK_Q:{ m_cam.move_up(move_speed);        } break;
        case SDLK_E:{ m_cam.move_down(move_speed);      } break;
    }

//    LOG_DBG("pos: {}",m_cam.pos);
//    LOG_DBG("origin: {}",m_cam.pos + m_cam.get_front());
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
            handle_key_down(e.key);
        }
        if (e.type == SDL_EVENT_MOUSE_MOTION){
            handle_mouse_motion(e.motion);
        }
        if (e.type == SDL_EVENT_MOUSE_WHEEL){
            handle_scroll_motion(e.wheel);
        }
    }
}
