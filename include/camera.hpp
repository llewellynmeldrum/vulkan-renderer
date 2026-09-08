#pragma once 
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"
#include "logger.hpp"
#include "types.hpp"
#include "glm/trigonometric.hpp"
struct Camera{
    static inline constexpr auto WORLD_UP = glm::vec3{0,1,0};
    f32 vfov = f32{60.0f};
    f32 zoom_sens = f32{0.5f};
    static constexpr auto znear = f32{0.01f};
    static constexpr auto zfar = f32{100.0f};
    glm::vec3 pos{6.4400353, 3.8162503, 4.4812794};
    glm::vec3 facing{};
    f32 yaw{170.0f},pitch{-10.0f};

    void handle_mouse_movement(glm::vec2 rel){
        rotate_yaw(rel.x);
        rotate_pitch(rel.y);
    }

    glm::vec3 get_facing() const{
        return glm::vec3{
            glm::cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            glm::sin(glm::radians(pitch)),
            glm::sin(glm::radians(yaw)) * cos(glm::radians(pitch)),
        };
    } 
    auto get_proj_matrix(f32 aspect)const{
        return glm::perspective(glm::radians(vfov), aspect, zfar,znear); 
    }
    auto get_view_matrix()       const{ return glm::lookAt(pos, pos + get_front(), WORLD_UP); }
    glm::vec3    get_up() const { return glm::vec3{0,1.0f,0}; }
    glm::vec3 get_right() const { return glm::cross(get_facing(), get_up());}
    glm::vec3 get_front() const { return glm::normalize(get_facing());}

    void rotate_pitch(f32 pitch_rel_01) {
//        LOG_DBG("pitch:{}",pitch);
        pitch -= pitch_rel_01 * vfov; 
        pitch = glm::clamp(pitch - pitch_rel_01* vfov, -89.0f, 89.0f); 
    }
    void rotate_yaw(f32 yaw_rel_01) { 
 //       LOG_DBG("yaw:{}",yaw);
        yaw = yaw + yaw_rel_01*vfov;
    }


    void move_right(f32 move_dist){
        pos += get_right() * move_dist;
    }
    void move_left(f32 move_dist){
        pos -= get_right() * move_dist;
    }
    void move_up(f32 move_dist){
        pos += get_up() * move_dist;
    }
    void move_down(f32 move_dist){
        pos -= get_up() * move_dist;
    }
    void move_forward(f32 move_dist){
        pos += get_facing() * move_dist;
    }
    void move_backward(f32 move_dist){
        pos -= get_facing() * move_dist;
    }

    void rotate_right(f32 rotate_dist){
        yaw += rotate_dist;
    }
    void rotate_left(f32 rotate_dist){
        yaw -= rotate_dist;
    }
    void rotate_up(f32 rotate_dist){
        pitch += rotate_dist;
    }
    void rotate_down(f32 rotate_dist){
        pitch -= rotate_dist;
    }
};
