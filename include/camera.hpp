#pragma once 
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"
#include "types.hpp"
#include "glm/trigonometric.hpp"
struct Camera{
    glm::vec3 pos = glm::vec3(2.0f);
    glm::vec3 facing{};
    f32 yaw{90.0f},pitch{0.0f};

    glm::vec3 get_facing(){
        return glm::vec3{
            glm::cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            glm::sin(glm::radians(pitch)),
            glm::sin(glm::radians(yaw)) * cos(glm::radians(pitch)),
        };
    }
    glm::vec3 get_up(){ return glm::vec3{0,1.0f,0}; }
    glm::vec3 get_right(){ return glm::cross(get_facing(), get_up());}
    glm::vec3 get_front(){ return glm::normalize(get_facing());}
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
        pos += get_front() * move_dist;
    }
    void move_backward(f32 move_dist){
        pos -= get_front() * move_dist;
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
