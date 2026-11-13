#pragma once 

#include "timer.hpp"
struct Profiler{
    static constexpr auto k_target_dt_seconds = 0.016;

    // NOTE: dt is the difference in time between the previous frames beginning, and the current frames beginning.
    // Essentially, the length of the previous frame.
    timer::duration dt = timer::seconds(k_target_dt_seconds); 
    std::size_t m_frame_count = 0;
    auto get_dt_seconds() -> f64{
        return timer::get_seconds(dt);
    }
    auto start_frame() -> void{
        current_frame_t0 = timer::now();
        dt = current_frame_t0 - prev_frame_t0;

    }
    auto end_frame() -> void{
        m_frame_count++;
        prev_frame_t0 = current_frame_t0;
    }
private:
    timer::time_point current_frame_t0;
    timer::time_point prev_frame_t0;
};
