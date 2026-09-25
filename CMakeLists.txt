cmake_minimum_required(VERSION 4.3.3)

project(vulkan_test0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
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
add_executable(vulkan_test0 
    "${APP_SRC}"
)
target_include_directories(vulkan_test0 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)


target_precompile_headers(vulkan_test0 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.hpp"
)

target_compile_options(vulkan_test0 PRIVATE
    -Wall
    -Wswitch                # Force errors like 'enum value x not evaluated in switch statement'
    -Werror                 
    -Wimplicit-fallthrough  # Why is this not part of -Wall
    -Wno-unused             # I cant stand this shit spamming my screen
    -Wunused-result         # These ones are okay though
#    -Wfatal-errors         # Gets annoying with clangd (i could disable it in .clangd but im lazy)
    -fdiagnostics-color=always
    -ffile-prefix-map=${CMAKE_SOURCE_DIR}=. # gives stack traces a relative path to cwd
)

set_target_properties( vulkan_test0 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

# TEMPORARY FOR BENCHMARKING COMPILE TIME
#target_compile_options(vulkan_test0 PRIVATE -ftime-report)


# =============================================================================
# LIBRARIES
# =============================================================================
include(FetchContent)

# VULKAN 
# ---------------------------------------
target_compile_definitions(vulkan_test0
    PUBLIC VULKAN_HPP_NO_CONSTRUCTORS
    PUBLIC VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
)
find_package(Vulkan REQUIRED)
target_link_libraries(vulkan_test0 PRIVATE
    Vulkan::Vulkan
)

# VMA (Vulkan Memory Allocator)
# ---------------------------------------
target_include_directories(vulkan_test0 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/VulkanMemoryAllocator/include"
)

# GLM (openGL Mathematics)
# ---------------------------------------
target_include_directories(vulkan_test0 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/OpenGL-Mathematics"
)
target_compile_definitions(vulkan_test0
    PUBLIC GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
    PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE
)

# SDL3
# ---------------------------------------
add_subdirectory(external/sdl3 EXCLUDE_FROM_ALL)
target_link_libraries(vulkan_test0 PRIVATE
    SDL3::SDL3
)

# CPPTRACE
# ---------------------------------------
if (ENABLE_CPPTRACE)
    target_include_directories(vulkan_test0 PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/external/cpptrace"
    )
    set(CPPTRACE_BUILD_SHARED   OFF)   # static ( no runtime .dylib bullshit)
    set(CPPTRACE_BUILD_TESTING  OFF)
    FetchContent_Declare(cpptrace   SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/cpptrace)
    FetchContent_MakeAvailable(cpptrace)
    target_link_libraries(vulkan_test0 PRIVATE cpptrace::cpptrace)
endif()

# LIBASSERT
# ---------------------------------------
target_include_directories(vulkan_test0 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/libassert"
)
add_subdirectory(external/libassert)
target_link_libraries(vulkan_test0 PRIVATE
    libassert::assert
)


# MAGIC ENUM
# ---------------------------------------
target_include_directories(vulkan_test0 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/magic_enum"
)
set(MAGIC_ENUM_OPT_BUILD_TESTS OFF)
set(MAGIC_ENUM_OPT_INSTALL     OFF)
FetchContent_Declare(magic_enum SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/magic_enum)


# REFLECT CPP
# ---------------------------------------
target_include_directories(vulkan_test0 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/reflect-cpp"
)
FetchContent_Declare(reflect_cpp SOURCE_DIR 
    "${CMAKE_SOURCE_DIR}/external/reflect-cpp"
)
FetchContent_MakeAvailable(reflect_cpp)
target_link_libraries(vulkan_test0 PRIVATE
    reflectcpp::reflectcpp
)

# IMGUI
# ---------------------------------------
target_include_directories(vulkan_test0 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui"
)
add_subdirectory(external/imgui)
target_link_libraries(vulkan_test0 PRIVATE
    imgui
)

# RANGE-V3 [HEADER ONLY]
# ---------------------------------------
target_include_directories(vulkan_test0 SYSTEM PRIVATE 
    "${CMAKE_SOURCE_DIR}/external/range-v3/include/"
)

# STB IMAGE  [HEADER ONLY]
# STB IMAGE_WRITE [HEADER ONLY]
# STB TRUETYPE    [HEADER ONLY]
# STB RECT_PACK   [HEADER ONLY]
# --------------------------------------
target_include_directories(vulkan_test0 SYSTEM PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/stb"
    # All are included under the stb directory
)

# FAST NOISE LITE [HEADER ONLY]
# --------------------------------------
target_include_directories(vulkan_test0 SYSTEM PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/FastNoiseLite"
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


add_dependencies(vulkan_test0 
    main_shaders
    shaders2d
)




# --- Optimization and sanitizers
OPTION(_ENABLE_ASAN  "ENABLE ADDRESSSANITIZER" OFF)
OPTION(_ENABLE_LSAN  "ENABLE ADDRESSSANITIZER LEAK DETECTION" OFF)
OPTION(_ENABLE_TSAN  "ENABLE THREADSANITIZER" OFF)
OPTION(_ENABLE_UBSAN "ENABLE UBSAN" OFF)
OPTION(_FAST_MATH    "ENABLE -FFAST-MATH" OFF)
OPTION(_O1 "ENABLE -O1" OFF)
OPTION(_O2 "ENABLE -O2" OFF)
OPTION(_O3 "ENABLE -O3" OFF)

if(_O1)
    target_compile_options(vulkan_test0 PRIVATE -O1)
endif()

if(_O2)
    target_compile_options(vulkan_test0 PRIVATE -O2)
endif()

if(_O3)
    target_compile_options(vulkan_test0 PRIVATE -O3)
endif()

if(_FAST_MATH)
    target_compile_options(vulkan_test0 PRIVATE -ffast-math)
endif()

if(_ENABLE_ASAN OR _ENABLE_LSAN)
    target_compile_options(vulkan_test0 PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options(vulkan_test0 PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
endif()

if(_ENABLE_TSAN)
    target_compile_options(vulkan_test0 PRIVATE -fsanitize=thread -fno-omit-frame-pointer -g)
    target_link_options(vulkan_test0 PRIVATE -fsanitize=thread -fno-omit-frame-pointer -g)
endif()

if(_ENABLE_UBSAN)
    target_compile_options(vulkan_test0 PRIVATE -fsanitize=undefined -fno-omit-frame-pointer -g)
    target_link_options(vulkan_test0 PRIVATE -fsanitize=undefined -fno-omit-frame-pointer -g)
endif()


add_custom_target(run
    COMMAND sh -c "./bin/vulkan_test0"
    DEPENDS vulkan_test0
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_custom_target(debug
    COMMAND lldb -o run -- "$<TARGET_FILE:vulkan_test0>"
    DEPENDS vulkan_test0
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    USES_TERMINAL
)

if(APPLE)
    if(ENABLE_CPPTRACE)
        add_custom_command(TARGET vulkan_test0 POST_BUILD
            COMMAND dsymutil $<TARGET_FILE:vulkan_test0>
            COMMENT "Generating .dSYM for stack traces for cpptrace/libassert"
        )
    endif()
endif()


