cmake_minimum_required (VERSION 3.14.4)
project(font_packer)

# Set C++ language version to C++17
set(CMAKE_C_STANDARD 99)

# Set relase/debug
set(CMAKE_BUILD_TYPE release)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set paths
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories(
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/../../lib"
)

file(GLOB SOURCES
  "src/main.c"
  # libs
  "../../lib/libschrift/schrift.c"
)

# Add the executable
add_executable(font_packer ${SOURCES})

# Set compiler flags
target_compile_options(font_packer PUBLIC -Wall -Wextra -o2 -g -fno-omit-frame-pointer)
target_link_options(font_packer PUBLIC -Wall -Wextra -o2 -g -fno-omit-frame-pointer)
