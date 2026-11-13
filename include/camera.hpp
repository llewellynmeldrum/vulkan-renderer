#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"
#include "logger.hpp"
#include "primitive_types.hpp"
#include "glm/trigonometric.hpp"
struct Camera{
    static inline constexpr auto WORLD_UP = glm::vec3{0,1,0};
    f32 vfov {60.0f}, vfov_min {20.0f}, vfov_max {120.0f};

    f32 zoom_sens {0.5f};
    f32 move_speed {1.0f};
    static constexpr auto znear = f32{0.01f};
    static constexpr auto zfar = f32{100.0f};
    glm::vec3 pos{6.4400353, 3.8162503, 4.4812794};
    glm::vec3 facing{};
    f32 yaw{170.0f},pitch{-10.0f};

    void look_around(glm::vec2 rel, f32 scale = 1.0f){
        rotate_yaw(rel.x, scale);
        rotate_pitch(rel.y, scale);
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

    auto move(glm::vec3 direction, f32 scale=1.0f){
        pos += direction * scale * move_speed;
    }
    void rotate_pitch(f32 pitch_rel_01, f32 scale) {
        pitch -= pitch_rel_01 * vfov; 
        pitch = glm::clamp(pitch - pitch_rel_01 * vfov * scale, -89.0f, 89.0f); 
    }
    void rotate_yaw(f32 yaw_rel_01, f32 scale) { 
        yaw = yaw + yaw_rel_01 * vfov * scale;
    }

    void zoom_to(f32 amount){
        vfov = std::clamp(amount,vfov_min,vfov_max);
    }
    void zoom_in(f32 multiplier){
        vfov = std::clamp(vfov*multiplier*zoom_sens,vfov_min,vfov_max);
    }
    void zoom_out(f32 multiplier){
        vfov = std::clamp(vfov*multiplier*zoom_sens,vfov_min,vfov_max);
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
