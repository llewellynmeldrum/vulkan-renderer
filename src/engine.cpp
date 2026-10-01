#include "engine.hpp"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_oldnames.h"
#include "color_utils.hpp"
#include "magic_enum.hpp"
#include <print>
auto Engine::get_instance() -> Engine*{
    ASSERT(instance, "Engine instance requested prior to init().");
    return Engine::instance;
}

auto Engine::init() -> void{
    if (instance){
        LOG_FATAL("Double initialization breh");
    }
    Engine::instance = this;
    static constexpr auto initialWindowLogicalExtent = glm::ivec2{
        1280,720
    };
    platform.init(initialWindowLogicalExtent );
    input.init(platform.get_window_handle(), initialWindowLogicalExtent);
    rend.init(platform.get_window_handle(),initialWindowLogicalExtent);
    world.init();



    upload_heightmap(world.m_heightmap);
    auto const mid = rend.m_windowLogicalExtent /2.0f ;
    rend.m_rend2d.set_draw_state( { .fill_color = make_rgba(0,255,0,255) });
    // screen center should be top left, with length of 200 lpx (logical pixels)
//    rend.m_rend2d.add_rect(mid + glm::vec2{-200,-200}, glm::vec2(400.0f));

    rend.m_rend2d.set_draw_state( { .fill_color = make_rgba(255,128,0,255) });
//    static constexpr size_t circle_count = 2;

    rend.m_rend2d.add_circle(mid, 200.0f);
    rend.m_rend2d.set_draw_state( { 
        .fill_color = make_rgba(255,200,255,255),
        .font_height = 128.0f,
    });
    rend.m_rend2d.add_text("Hello World", mid);
    rend.upload_mesh2d();
}

void Engine::cleanup(){
    ASSERT(instance, "cleanup() called before init()" );
    rend.cleanup();
    platform.cleanup();

}
void Engine::run(){
    using namespace std::chrono_literals;
    while (!m_shouldQuit) {
        poll_events();
        handle_input_actions();
        if (m_windowResized){
            handle_window_resize();
        }
        if (!m_worldUpdatesPaused){
            world.per_frame_update();
        }
        if (m_shouldRender) {
            ui_draw();
            rend.draw(cam);
           // use sdl builtins and draw a circle, have a define mode or something to switch to 2d, or just make a new project really quickk (better idea) SDL_RenderRect(SDL_GetRendererr) ;
        } else{
            std::this_thread::sleep_for(100ms);
        }
    }
}

auto Engine::upload_heightmap(Heightmap const& heightmap) -> void{
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
    rend.upload_mesh3d(mesh_id, cpu_mesh, model);
}
