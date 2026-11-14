#pragma once

// NOTE: Precompiled header
// WARNING: Dont put non library/STL headers in here. Only headers which are very rarely/never modified belong here.

// STL HEADERS
#if defined(__cpp_impl_reflection)
#include <meta>
#endif
#include <memory>
#include <unordered_map>
#include <mdspan>


#include <cstddef>
#include <cmath>
#include <cstdlib>
#include <cstdint>

#include <string>
#include <ranges>
#include <format>
#include <vector>
#include <flat_map>
#include <string_view>
#include <optional>
#include <utility>
#include <type_traits>
#include <concepts>
#include <print>
#include <numeric>
#include <iostream>
#include <array>

#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <cstdio>
#include <algorithm>
#include <tuple>
#include <stack>
#include <span>
#include <ostream>
#include <expected>
#include <deque>
#include <chrono>
#include <charconv>




// LIBRARY HEADERS
#include <imgui.h>
#include <libassert/assert.hpp>
#include <toml11/parser.hpp>
#include <SDL3/SDL.h>
#include <cpptrace/utils.hpp>
#include <cpptrace/cpptrace.hpp>
#include <cpptrace/basic.hpp>
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
