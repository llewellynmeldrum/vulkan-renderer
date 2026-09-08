#pragma once 
#include <thread>

#include "platform.hpp"
#include "renderer.hpp"
#include "input.hpp"
#include "world.hpp"
#include "camera.hpp"

#include "shared_transformations.hpp"
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

    bool m_shouldRender         {true};
    bool m_windowResized        {false};
    bool m_worldUpdatesPaused   {false};
    bool m_shouldQuit           {false};

    void upload_heightmap(Heightmap const& heightmap){
        i32 samples_per_meter = 1;
        auto cpu_mesh = mesh_heightmap(
            heightmap,
            HeightMapMeshCreateInfo{
                .num_x_samples = static_cast<u32>(samples_per_meter * heightmap.m_extentX),
                .num_z_samples = static_cast<u32>(samples_per_meter * heightmap.m_extentZ),
            }
        );
        static constexpr MeshID mesh_id = 0;

        auto model = glm::mat4x4(1.0f);
        model = glm::translate(model,heightmap.m_world_center);
        rend.upload_mesh(mesh_id, cpu_mesh, model);
    }

 private:
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
