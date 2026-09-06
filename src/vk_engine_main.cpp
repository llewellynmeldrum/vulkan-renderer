#include "vk_engine.hpp"
#include <thread>

void VkEngine::run() {
    using namespace std::chrono_literals;
    while (!m_shouldStopRunning) {
        handle_inputs();
        if (m_shouldStopRendering) {
            std::this_thread::sleep_for(100ms);
            continue;
        }
        per_frame_update();
        draw();
    }
}
