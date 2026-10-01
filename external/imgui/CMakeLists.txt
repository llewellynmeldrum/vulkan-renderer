cmake_minimum_required(VERSION 3.28)

add_library(imgui STATIC
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp

    backends/imgui_impl_sdl3.cpp
    backends/imgui_impl_vulkan.cpp
    misc/cpp/imgui_stdlib.cpp

    # WARNING: demo window TU is excluded by default
    imgui_demo.cpp
)


target_include_directories(imgui PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/backends"
    "${CMAKE_CURRENT_SOURCE_DIR}/misc/cpp"
)
target_link_libraries(imgui PRIVATE libassert::assert)
target_link_libraries(imgui PUBLIC SDL3::SDL3 Vulkan::Vulkan)
target_compile_features(imgui PUBLIC cxx_std_23)

# disable imgui warnings
target_compile_options(imgui PRIVATE
    -w
)
