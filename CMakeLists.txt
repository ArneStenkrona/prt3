cmake_minimum_required (VERSION 3.14.4)
project (prt3)

# Set C++ language version to C++17
set (CMAKE_CXX_STANDARD 17)

# Set relase/debug
set(CMAKE_BUILD_TYPE release)

# Set paths
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}"
)

file(GLOB SOURCES
"src/main/main.cpp"
"src/engine/scene/node.cpp"
"src/engine/scene/scene.cpp"
"src/engine/rendering/model_manager.cpp"
"src/engine/rendering/model.cpp"
"src/engine/rendering/renderer.cpp"
"src/engine/core/context.cpp"
"src/engine/core/engine.cpp"
)

## Libraries ##

# glm
find_package(glm REQUIRED)
# assimp
find_package(ASSIMP REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIR})

## Build ###
# Add the executable
add_executable(prt3 ${SOURCES})
# Set compiler flags
target_compile_options(prt3 PUBLIC -Wall -Wextra -o2 -g -fsanitize=address)
target_link_options(prt3 PUBLIC -Wall -Wextra -o2 -g -fsanitize=address)
# Link libraries
target_link_libraries(prt3 glm)
target_link_libraries(prt3 assimp::assimp)
