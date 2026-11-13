#pragma once
#include "Gui.hpp"
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
#include <thread>

#include "platform.hpp"
#include "renderer.hpp"
#include "input.hpp"
#include "world.hpp"
#include "camera.hpp"
#include "profiler.hpp"
#include "input_actions.hpp"

#include "transformations.hpp"
#include "mesh_id.hpp"

struct Engine{
 public:
    Engine();
    auto loop() -> void;

    static constexpr auto initialWindowLogicalExtent = glm::ivec2{
        1280,720
    };

    Platform platform;
    InputState input_state;
    Renderer3D r3d;
    Renderer2D r2d;
    World world;
    Camera cam;
    Gui ui;
    Profiler profiler;


    std::vector<Action> actions;
    auto should_run() const{ return m_shouldExit; }
    bool m_shouldExit{false};
};
//Gui::Gui(Engine &eng) {}
