#include "engine.hpp"
#include "renderer2d.hpp"
#include "font_atlas.hpp"
int main() {

    timer::set_prog_epoch();
    cpptrace::register_terminate_handler();
    {
        auto engine = Engine{};
        engine.init();
        engine.run();
        engine.cleanup();
    }
}
