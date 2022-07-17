cmake_minimum_required (VERSION 3.14.4)
project (prt3)

# Set C++ language version to C++17
set (CMAKE_CXX_STANDARD 17)

# Set relase/debug
set(CMAKE_BUILD_TYPE release)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Remove previously built .js-file
# (necessary to force regeneration of assets)
file(REMOVE "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.js")

# Set paths
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}"
)

file(GLOB SOURCES
"src/main/main.cpp"
"src/driver/opengl/gl_material.cpp"
"src/driver/opengl/gl_mesh.cpp"
"src/driver/opengl/gl_renderer.cpp"
"src/driver/opengl/gl_utility.cpp"
"src/engine/core/context.cpp"
"src/engine/core/engine.cpp"
"src/engine/core/input.cpp"
"src/engine/rendering/camera.cpp"
"src/engine/rendering/model_manager.cpp"
"src/engine/rendering/model.cpp"
"src/engine/rendering/renderer.cpp"
"src/engine/scene/node.cpp"
"src/engine/scene/component_manager.cpp"
"src/engine/scene/scene.cpp"
)

add_subdirectory(lib/glm EXCLUDE_FROM_ALL)
add_subdirectory(lib/assimp EXCLUDE_FROM_ALL)

# Add the executable
add_executable(prt3 ${SOURCES})

add_custom_target(copy_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/
)
add_dependencies(prt3 copy_resources)

# Wasm specific
if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    set(CMAKE_CXX_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2")
    set_target_properties(prt3 PROPERTIES LINK_FLAGS "${CMAKE_LINK_FLAGS} -std=c++11 -MMD -MP -Oz -s USE_SDL=2 --use-preload-plugins -s USE_SDL_IMAGE=2 -s WASM=1 -sLLD_REPORT_UNDEFINED -sASSERTIONS --embed-file assets -sALLOW_MEMORY_GROWTH -s EXCEPTION_DEBUG -sNO_DISABLE_EXCEPTION_CATCHING -s SDL2_IMAGE_FORMATS='[\"bmp\",\"png\"]' -sFULL_ES2")
endif ()

# Set compiler flags
target_compile_options(prt3 PUBLIC -Wall -Wextra -o2 -g -fsanitize=address)
target_link_options(prt3 PUBLIC -Wall -Wextra -o2 -g -fsanitize=address)
# Link libraries
target_link_libraries(prt3 assimp::assimp)
target_link_libraries(prt3 glm)
target_link_libraries(prt3 SDL2)
