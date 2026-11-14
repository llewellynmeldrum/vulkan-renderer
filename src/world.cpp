#include "world.hpp"
#include "cppslop.hpp"
#include "logger.hpp"
#include "stable_type_name.hpp"

World::World(){
    LOG_DBG("Initializing {} ", stable_type_name<THIS_TYPE>());
}
