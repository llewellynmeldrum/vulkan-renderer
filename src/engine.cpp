#include "engine.hpp"
#include "SDL3/SDL_events.h"
#include "magic_enum.hpp"
#include <print>
auto Engine::init()
-> void{
    static constexpr auto initialWindowLogicalExtent = glm::ivec2{
        1280,720
    };
    platform.init(initialWindowLogicalExtent );
    input.init(platform.get_window_handle(), initialWindowLogicalExtent);
    rend.init(platform.get_window_handle(),initialWindowLogicalExtent);
    world.init();
    upload_heightmap(world.m_heightmap);
}

void Engine::cleanup(){
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
            rend.draw(cam);
        } else{
            std::this_thread::sleep_for(100ms);
        }
    }
}

