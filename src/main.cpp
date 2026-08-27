#include "vk_engine.hpp"

int main() {
    timer::set_prog_epoch();
    cpptrace::register_terminate_handler();
    {
        VkEngine engine{};
        engine.run();
    }
    LOG_EXIT(EXIT_SUCCESS);
}
