#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
#include <thread>

#include "platform.hpp"
#include "r3d.hpp"
#include "input.hpp"
#include "world.hpp"
#include "camera.hpp"
#include "profiler.hpp"
#include "input_actions.hpp"

#include "transformations.hpp"
#include "mesh_id.hpp"
#include "Gui.hpp"
#include "input_binds.hpp"

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
    std::vector<Binding> bindings{
        Binding{ 
            .type = HoldButtonBind{
                .button = ButtonCode::KEY_W,
                .action = {ActionType::MoveCamera},
            }
        }
    };


    std::vector<Action> actions;
    auto should_run() const{ return !m_shouldExit; }
    bool m_shouldExit{false};
};

// SECTION: Free functions which act on one or more parts of the engine.
auto post_process_actions(std::vector<Action>& out_actions) -> void;

auto derive_actions_from_input(
    std::vector<Action>& out_actions,
    InputState const& input,
    std::span<const Binding> bindings
) -> std::vector<Action>&;
