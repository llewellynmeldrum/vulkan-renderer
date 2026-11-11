set(GCC_VERSION 16 CACHE STRING "Homebrew GCC major version")
if(NOT DEFINED GCC_BIN_DIR)
  set(GCC_BIN_DIR "/opt/homebrew/bin")
endif()
set(CMAKE_C_COMPILER   "${GCC_BIN_DIR}/gcc-${GCC_VERSION}")
set(CMAKE_CXX_COMPILER "${GCC_BIN_DIR}/g++-${GCC_VERSION}")

if(NOT DEFINED CMAKE_OSX_SYSROOT OR CMAKE_OSX_SYSROOT STREQUAL "")
  execute_process(COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE _macos_sdk OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(CMAKE_OSX_SYSROOT "${_macos_sdk}" CACHE PATH "macOS SDK for Homebrew GCC")
endif()

# Take GCC's default search list, drop include-fixed (stale SDK14 copies), pass the rest explicitly.
function(_gcc_clean_system_includes lang compiler out_var)
  execute_process(
    COMMAND "${compiler}" -E -v -x ${lang} /dev/null -isysroot "${CMAKE_OSX_SYSROOT}"
    ERROR_VARIABLE _v OUTPUT_QUIET)
  string(REGEX REPLACE "^.*#include <...> search starts here:\n(.*)\nEnd of search list.*$" "\\1" _v "${_v}")
  string(REPLACE "\n" ";" _dirs "${_v}")
  set(_flags "-isysroot ${CMAKE_OSX_SYSROOT}")
  foreach(_d IN LISTS _dirs)
    string(STRIP "${_d}" _d)
    if(_d STREQUAL "" OR _d MATCHES "include-fixed$" OR _d MATCHES "/Frameworks")
      continue()
    endif()
    get_filename_component(_d "${_d}" REALPATH)
    string(APPEND _flags " -isystem ${_d}")
  endforeach()
  set(${out_var} "${_flags}" PARENT_SCOPE)
endfunction()

_gcc_clean_system_includes(c++ "${CMAKE_CXX_COMPILER}" _cxx_sys)
_gcc_clean_system_includes(c   "${CMAKE_C_COMPILER}"   _c_sys)
set(CMAKE_CXX_FLAGS_INIT "${_cxx_sys}")
set(CMAKE_C_FLAGS_INIT   "${_c_sys}")
