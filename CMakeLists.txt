cmake_minimum_required(VERSION 4.3.3)

set(APP_NAME vmc)
project(${APP_NAME} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Tell CMake to look inside the 'cmake' folder for module files
list(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

file( GLOB_RECURSE APP_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${APP_NAME} 
    "${APP_SRC}"
)
target_include_directories(${APP_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# -- PROJECT SPECIFIC MACROS
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp" 
PROPERTIES COMPILE_DEFINITIONS "PERMIT_ENGINE_HPP_INCLUDE=1")

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/gui.cpp" 
PROPERTIES COMPILE_DEFINITIONS "PERMIT_ENGINE_HPP_INCLUDE=1")

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/engine.cpp" 
PROPERTIES COMPILE_DEFINITIONS "PERMIT_ENGINE_HPP_INCLUDE=1")

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/include/gui.hpp" 
PROPERTIES COMPILE_DEFINITIONS "PERMIT_ENGINE_HPP_INCLUDE=1")

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/include/engine.hpp" 
PROPERTIES COMPILE_DEFINITIONS "PERMIT_ENGINE_HPP_INCLUDE=1")


target_compile_options(${APP_NAME} PRIVATE
    -Wall
    -Wswitch                # Force errors like 'enum value x not evaluated in switch statement'
    -Wimplicit-fallthrough  # Why is this not part of -Wall
    -Wno-unused             # I cant stand this shit spamming my screen
    -Wunused-result         # These ones are okay though
#    -Wfatal-errors         # Gets annoying with clangd (i could disable it in .clangd but im lazy)
    -fdiagnostics-color=always
    -ffile-prefix-map=${CMAKE_SOURCE_DIR}=. # gives stack traces a relative path to cwd
    -freflection
)

OPTION(_TIME_REPORT "Print -ftime-report for each TU" OFF)

set(_MAX_ERRORS 0 CACHE STRING "Stop each TU after N errors (0 = no limit, keeps -Werror)")
if(_TIME_REPORT)
    target_compile_options(${APP_NAME} PRIVATE -ftime-report)
endif()

if(_MAX_ERRORS GREATER 0)
    target_compile_options(${APP_NAME} PRIVATE
        -fmax-errors=${_MAX_ERRORS}
        -fconcepts-diagnostics-depth=2
        # -fno-diagnostics-show-nesting-locations   # more compact, loses some file names
    )
else()
    target_compile_options(${APP_NAME} PRIVATE -Werror)
endif()
#set_target_properties(${APP_NAME} PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
#)

# =============================================================================
# LIBRARIES
# =============================================================================
include(FetchContent)

# VULKAN 
# ---------------------------------------
target_compile_definitions("${APP_NAME}"
    PUBLIC VULKAN_HPP_NO_CONSTRUCTORS
    PUBLIC VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
)
find_package(Vulkan REQUIRED)
target_link_libraries("${APP_NAME}" PRIVATE
    Vulkan::Vulkan
)

# VMA (Vulkan Memory Allocator)
# ---------------------------------------
target_include_directories("${APP_NAME}" SYSTEM PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/VulkanMemoryAllocator/include"
)

# GLM (openGL Mathematics)
# ---------------------------------------
target_include_directories("${APP_NAME}" SYSTEM PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/OpenGL-Mathematics"
)
target_compile_definitions("${APP_NAME}"
    PUBLIC GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
    PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE
)

# SDL3
# ---------------------------------------
#NOTE: SDL3 is the only non vendored library (other than vulkan)

# CPPTRACE
# ---------------------------------------
if (ENABLE_CPPTRACE)
    target_include_directories("${APP_NAME}" SYSTEM PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/external/cpptrace"
    )
    set(CPPTRACE_BUILD_SHARED   OFF)   # static ( no runtime .dylib bullshit)
    set(CPPTRACE_BUILD_TESTING  OFF)
    FetchContent_Declare(cpptrace   SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/cpptrace)
    FetchContent_MakeAvailable(cpptrace)
    target_link_libraries("${APP_NAME}" PRIVATE cpptrace::cpptrace)
endif()

# LIBASSERT
# ---------------------------------------
target_include_directories("${APP_NAME}" SYSTEM PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/libassert"
)
add_subdirectory(
    external/libassert
    SYSTEM
)
target_link_libraries("${APP_NAME}" PRIVATE
    libassert::assert
)


# MAGIC ENUM
# ---------------------------------------
target_include_directories("${APP_NAME}" SYSTEM PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/magic_enum"
)
set(MAGIC_ENUM_OPT_BUILD_TESTS OFF)
set(MAGIC_ENUM_OPT_INSTALL     OFF)
FetchContent_Declare(magic_enum SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/magic_enum)


# REFLECT CPP
# ---------------------------------------
target_include_directories("${APP_NAME}" SYSTEM PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/reflect-cpp"
)
FetchContent_Declare(reflect_cpp SOURCE_DIR 
    "${CMAKE_SOURCE_DIR}/external/reflect-cpp"
)
FetchContent_MakeAvailable(reflect_cpp)
target_link_libraries("${APP_NAME}" PRIVATE
    reflectcpp::reflectcpp
)

# IMGUI
# ---------------------------------------
target_include_directories("${APP_NAME}" SYSTEM PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/misc/cpp"
)
add_subdirectory(
    external/imgui
    SYSTEM
)
target_link_libraries("${APP_NAME}" PRIVATE
    imgui
)

# IMPLOT
# ---------------------------------------
target_include_directories(${APP_NAME} SYSTEM PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/implot"
)
add_subdirectory(
    external/implot
    SYSTEM
)
target_link_libraries(${APP_NAME} PRIVATE
    implot
)


# STB IMAGE  [HEADER ONLY]
# STB IMAGE_WRITE [HEADER ONLY]
# STB TRUETYPE    [HEADER ONLY]
# STB RECT_PACK   [HEADER ONLY]
# --------------------------------------
target_include_directories("${APP_NAME}" SYSTEM PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/stb"
    # All are included under the stb directory
)

# FAST NOISE LITE [HEADER ONLY]
# --------------------------------------
target_include_directories("${APP_NAME}" SYSTEM PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/FastNoiseLite"
)

# toml11 
# ---------------------------------------
target_include_directories(${APP_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/toml11/include"
)
FetchContent_Declare(toml11 SOURCE_DIR 
    "${CMAKE_SOURCE_DIR}/external/toml11"
)
set(TOML11_PRECOMPILE ON)
FetchContent_MakeAvailable(toml11 )
target_link_libraries(${APP_NAME} PRIVATE toml11::toml11)

target_precompile_headers(${APP_NAME} PRIVATE
   "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.hpp" 
)

# SLANG SHADER SUPPORT 
# --------------------------------------
include(SlangShaderTargets)
add_slang_shader_target(
    main_shaders

    SOURCE
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/main_shaders.slang"

    ENTRY_POINTS
        vertMain 
        fragMain 
        vertWireframe 
        fragWireframe
)

add_slang_shader_target(
    shaders2d

    SOURCE
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shaders2d.slang"

    ENTRY_POINTS
        vertMain 
        fragMain 
)

add_slang_shader_target(
    voxel_dda

    SOURCE
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/voxel_dda.slang"

    ENTRY_POINTS
        vertMain 
        fragMain 
)

add_dependencies("${APP_NAME}" 
    voxel_dda
    main_shaders
    shaders2d
)




# --- Optimization and sanitizers
OPTION(_ENABLE_ASAN  "ENABLE ADDRESSSANITIZER" OFF)
OPTION(_ENABLE_LSAN  "ENABLE ADDRESSSANITIZER LEAK DETECTION" OFF)
OPTION(_ENABLE_TSAN  "ENABLE THREADSANITIZER" OFF)
OPTION(_ENABLE_UBSAN "ENABLE UBSAN" OFF)
OPTION(_FAST_MATH    "ENABLE -FFAST-MATH" OFF)
OPTION(_O0 "ENABLE -O0" OFF)
OPTION(_O1 "ENABLE -O1" OFF)
OPTION(_O2 "ENABLE -O2" OFF)
OPTION(_O3 "ENABLE -O3" OFF)

if(_O0)
    target_compile_options(${APP_NAME} PRIVATE -O0)
endif()
if(_O1)
    target_compile_options("${APP_NAME}" PRIVATE -O1)
endif()

if(_O2)
    target_compile_options("${APP_NAME}" PRIVATE -O2)
endif()

if(_O3)
    target_compile_options("${APP_NAME}" PRIVATE -O3)
endif()

if(_FAST_MATH)
    target_compile_options("${APP_NAME}" PRIVATE -ffast-math)
endif()

if(_ENABLE_ASAN OR _ENABLE_LSAN)
    target_compile_options("${APP_NAME}" PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options("${APP_NAME}" PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
endif()

if(_ENABLE_TSAN)
    target_compile_options("${APP_NAME}" PRIVATE -fsanitize=thread -fno-omit-frame-pointer -g)
    target_link_options("${APP_NAME}" PRIVATE -fsanitize=thread -fno-omit-frame-pointer -g)
endif()

if(_ENABLE_UBSAN)
    target_compile_options("${APP_NAME}" PRIVATE -fsanitize=undefined -fno-omit-frame-pointer -g)
    target_link_options("${APP_NAME}" PRIVATE -fsanitize=undefined -fno-omit-frame-pointer -g)
endif()



if(APPLE)
    find_program(DSYMUTIL dsymutil HINTS /usr/bin NO_DEFAULT_PATH)
    if(ENABLE_CPPTRACE)
        add_custom_command(TARGET ${APP_NAME} POST_BUILD
            COMMAND ${DSYMUTIL} $<TARGET_FILE:${APP_NAME}>
            COMMENT "Generating .dSYM for stack traces for cpptrace/libassert"
        )
    endif()
endif()


