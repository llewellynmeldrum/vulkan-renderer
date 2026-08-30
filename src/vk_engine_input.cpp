

#include <SDL3/SDL.h>

#include "vk_engine.hpp"
#include "format_specs.hpp"

void VkEngine::handle_key_down(SDL_KeyboardEvent const& key_ev){
    auto rotate_speed = f32{1.0f};
    switch(key_ev.key){
        case SDLK_T:{
            m_vkPolygonMode = m_vkPolygonMode == vk::PolygonMode::eFill ? vk::PolygonMode::eLine : vk::PolygonMode::eFill;
        } break;

        case SDLK_LEFT:{
            m_cam.rotate_left(rotate_speed);
        } break;
        case SDLK_RIGHT:{
            m_cam.rotate_right(rotate_speed);
        } break;
        case SDLK_UP:{
            m_cam.rotate_up(rotate_speed);
        } break;
        case SDLK_DOWN:{
            m_cam.rotate_down(rotate_speed);
        } break;
        case SDLK_A:{
            m_cam.move_left(0.1f);
        } break;
        case SDLK_D:{
            m_cam.move_right(0.1f);
        } break;
        case SDLK_W:{
            m_cam.move_forward(0.1f);
            //m_camPos.z -= 0.1f;
        } break;
        case SDLK_S:{
            m_cam.move_backward(0.1f);
            //m_camPos.z += 0.1f;
        } break;
        case SDLK_Q:{
            m_cam.move_up(0.1f);
            //m_camPos.z += 0.1f;
        } break;
        case SDLK_E:{
            m_cam.move_down(0.1f);
            //m_camPos.z += 0.1f;
        } break;
    }

    LOG_DBG("pos: {}",m_cam.pos);
    LOG_DBG("origin: {}",m_cam.pos + m_cam.get_front());
}
void VkEngine::handle_inputs(){
    SDL_Event e{};
    while ((SDL_PollEvent(&e)) != 0) {
        if (e.type == SDL_EVENT_QUIT) {
            m_shouldStopRunning= true;
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
    }
}
