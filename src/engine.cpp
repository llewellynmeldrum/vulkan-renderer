#include <list>
#include <print>
#include <utility>
#include <variant>
#include <span>

#include "glm_types.hpp"
#include "engine.hpp"
#include "Engine_Events.hpp"
#include "Engine_Gui.hpp"

#include "color_utils.hpp"
#include "timer.hpp"
#include "auto_format.hpp"

#include "format_specs.hpp"
#include "input_binds.hpp"

void apply_actions_to_camera(std::span<Action const> actions, Camera & cam, f64 dt_s){
    // since dt is in seconds, the scale of the action is essentially the 'units per second'.
    for (auto const& act: actions){
        LOG_DBG(
            "Applying action '{}', {} {}",
            act.type,
            act.delta2==glm::vec2{0,0} ? "" : std::format("{}, ", act.delta2),
            act.delta3==glm::vec3{0,0,0} ? "" : std::format("{}", act.delta3)
        );
        auto scale = dt_s;
        switch (act.type){
            case ActionType::MoveCamera:   cam.move(act.get_delta3(), scale);       break;

            case ActionType::LookAround:   cam.look_around(act.get_delta2()); break;

            case ActionType::ZoomIn:       cam.zoom_by(scale);          break;
            case ActionType::ZoomOut:      cam.zoom_by(scale);          break;
        }
    }
}

struct FrameContext{};
auto begin_frame() -> FrameContext{
    return {};
}

void Engine::loop(){
    profiler.start_frame();
    auto dt_s = profiler.get_dt_seconds();
    if (ui.wants_mouse()){
        platform.uncapture_cursor();
    }else{
        platform.capture_cursor();
    }

    auto events = poll_events(platform);        // Platform polls for events
    input_state = update_input_states(                         // input is updated to reflect the events 
        events, 
        input_state, 
        ui.wants_mouse(), 
        ui.wants_keyboard()
    );
    actions = derive_actions_from_input(actions, input_state, bindings); // based on input, derive actions
    post_process_actions(actions);                            // filter actions
    apply_actions_to_camera(actions, cam, dt_s);              // apply the actions.

    /*
    LOG_DBG("End of frame {} - frametime: {:g}ms",
    profiler.m_frameCount, timer::to_milliseconds(profiler.dt));
    */


//    auto frame = begin_frame();
//    auto frame = begin_frame();
//    draw_world(r3d,world);
//    draw_ui(r2d,ui);
    input_state.end_frame();
    profiler.end_frame();
}




Engine::Engine()
    : platform(initialWindowLogicalExtent)  // Creates window
    , input_state()                         // input state
    , r3d(platform.get_window_handle(),initialWindowLogicalExtent,platform.m_pixelDensity)
    , world()
    , cam()
    , ui(platform.get_window_handle())
{
    init_gui(ui, platform, r3d);
}
