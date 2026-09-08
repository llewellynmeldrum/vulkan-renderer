#pragma once 
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_video.h"
#include "glm/vec2.hpp"
#include "input_keycodes.hpp"
#include "logger.hpp"
#include "common_concepts.hpp"
#include "enum_map.hpp"
#include "shared_transformations.hpp"


struct Input{
public:
    SDL_Window*                     m_window{nullptr};
    glm::vec2                       m_windowLogicalExtent{};
    glm::vec2                       m_windowPixelExtent{};
    static constexpr glm::vec2      m_windowNdcExtent{2, 2};
    f32 m_pixelSize{0.0f};
    std::size_t frame_count{0};

    void init(SDL_Window* window, glm::uvec2 windowLogicalExtent) {
        m_pixelSize = SDL_GetWindowPixelDensity(window);
        m_window = window;
        handle_window_resize(windowLogicalExtent);
    }

    void end_frame() {
        // cur still holds whatever has been pressed down, 
        // so we can continuously query for held keys.
        prev = cur;
        cur.mouse_move_logical= {};
        cur.mouse_move_pixel = {};
        cur.mouse_move_ndc= {};
    }

    void report_keydown(SDL_KeyboardEvent e){
        cur.key_down_state[static_cast<KeyCode>(e.scancode)] = true;
    }
    void report_keyup(SDL_KeyboardEvent e){
        cur.key_down_state[static_cast<KeyCode>(e.scancode)] = false;
    }

    void report_mouse_moved(SDL_MouseMotionEvent e){
        auto const pLogical = glm::vec2{e.x,e.y};
        cur.mouse_pos_logical = pLogical;
        cur.mouse_pos_pixel = logical_to_pixel(pLogical);
        cur.mouse_pos_ndc = logical_to_ndc(pLogical);

        auto const pLogicalDiff = glm::vec2{e.xrel, e.yrel};
        cur.mouse_move_logical = pLogicalDiff;
        cur.mouse_move_pixel = logical_to_pixel(pLogicalDiff);
        cur.mouse_move_ndc = logical_to_ndc(pLogicalDiff);
    }
    void report_scroll(SDL_MouseWheelEvent e){
        cur.mouse_scroll_d = glm::vec2{e.x,e.y};
    }

    bool key_down(KeyCode code) const{
        return cur.key_down_state[code];
    }
    bool modifier_enabled(KeyModBits mod) const{
        return cur.key_mod_state.has(mod);
    }

    template<typename ...Args>
        requires variadic::all_same<Args...>
    bool any_keys_down(Args... code) const{
        return (key_down(code) || ...);
    }

    template<typename ...Args>
        requires variadic::all_same<Args...>
    bool all_keys_down(Args... code) const{
        return (key_down(code) && ...);
    }


    bool just_pressed(KeyCode key) const{
        bool pressed_last_frame = prev.key_down_state[key];
        bool pressed_this_frame = cur.key_down_state[key];

        return !pressed_last_frame && pressed_this_frame;
    }

    struct State{
        EnumMap<KeyCode, bool> key_down_state{};
        KeyMod key_mod_state;

        glm::vec2 mouse_pos_ndc{};
        glm::vec2 mouse_pos_pixel{};
        glm::vec2 mouse_pos_logical{};

        glm::vec2 mouse_scroll_d{};

        glm::vec2 mouse_move_ndc{};
        glm::vec2 mouse_move_pixel{};
        glm::vec2 mouse_move_logical{};

    };

    glm::vec2 mouse_pos_logical_movement() const {
        return cur.mouse_move_logical;
    }
    glm::vec2 mouse_pos_ndc_movement() const {
        return cur.mouse_move_ndc;
    }
    glm::vec2 mouse_pos_pixel_movement() const {
        return cur.mouse_move_pixel;
    }

    State cur;
    State prev;


    void handle_window_resize(glm::vec2 windowLogicalExtent){
        m_windowLogicalExtent = {windowLogicalExtent.x,windowLogicalExtent.y};
        m_windowPixelExtent = logical_to_pixel(windowLogicalExtent);
    }
private:
    glm::vec2 logical_to_ndc(glm::vec2 pLogical)const { return detail::logical_to_ndc(pLogical, m_windowLogicalExtent);}
    glm::vec2 ndc_to_logical(glm::vec2 pNdc)const { return detail::ndc_to_logical(pNdc, m_windowLogicalExtent);}
    glm::vec2 logical_to_pixel(glm::vec2 pLogical)const { return detail::logical_to_pixel(pLogical,m_pixelSize);}
    glm::vec2 pixel_to_logical(glm::vec2 pPixel)const { return detail::pixel_to_logical(pPixel,m_pixelSize);}
};
