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
  "${PROJECT_SOURCE_DIR}/lib/imgui"
  "${PROJECT_SOURCE_DIR}/lib/imgui-filebrowser"
  "${PROJECT_SOURCE_DIR}/lib/ImGuizmo"
  "${PROJECT_SOURCE_DIR}/lib/hash-library"
)

file(GLOB SOURCES
"src/main/main.cpp"
"src/driver/opengl/gl_material_manager.cpp"
"src/driver/opengl/gl_material.cpp"
"src/driver/opengl/gl_mesh.cpp"
"src/driver/opengl/gl_model_manager.cpp"
"src/driver/opengl/gl_postprocessing_chain.cpp"
"src/driver/opengl/gl_postprocessing_pass.cpp"
"src/driver/opengl/gl_renderer.cpp"
"src/driver/opengl/gl_texture_manager.cpp"
"src/driver/opengl/gl_shader_utility.cpp"
"src/driver/opengl/gl_shader.cpp"
"src/driver/opengl/gl_source_buffers.cpp"
"src/driver/opengl/gl_utility.cpp"
"src/engine/animation/animation_system.cpp"
"src/engine/component/script/script.cpp"
"src/engine/component/animated_model.cpp"
"src/engine/component/collider_component.cpp"
"src/engine/component/component_manager.cpp"
"src/engine/component/material.cpp"
"src/engine/component/mesh.cpp"
"src/engine/component/model.cpp"
"src/engine/component/point_light.cpp"
"src/engine/component/script_set.cpp"
"src/engine/physics/aabb_tree.cpp"
"src/engine/physics/aabb.cpp"
"src/engine/physics/collider.cpp"
"src/engine/physics/gjk.cpp"
"src/engine/physics/physics_system.cpp"
"src/engine/core/context.cpp"
"src/engine/core/engine.cpp"
"src/engine/core/input.cpp"
"src/engine/editor/action/action_add_node.cpp"
"src/engine/editor/action/action_manager.cpp"
"src/engine/editor/action/action_remove_node.cpp"
"src/engine/editor/action/action_transform_node.cpp"
"src/engine/editor/gizmo/gizmo_manager.cpp"
"src/engine/editor/gui_components/editor_gui.cpp"
"src/engine/editor/gui_components/menu.cpp"
"src/engine/editor/gui_components/node_inspector.cpp"
"src/engine/editor/gui_components/panel.cpp"
"src/engine/editor/gui_components/scene_hierarchy.cpp"
"src/engine/editor/editor_camera.cpp"
"src/engine/editor/editor_context.cpp"
"src/engine/editor/editor.cpp"
"src/engine/geometry/shapes.cpp"
"src/engine/rendering/camera.cpp"
"src/engine/rendering/material_manager.cpp"
"src/engine/rendering/model_manager.cpp"
"src/engine/rendering/model.cpp"
"src/engine/rendering/renderer.cpp"
"src/engine/scene/node.cpp"
"src/engine/scene/scene.cpp"
"src/engine/scene/script_container.cpp"
"src/engine/scene/transform_cache.cpp"
"src/util/checksum.cpp"
"src/util/file_util.cpp"
# libs
"lib/imgui/*.cpp"
"lib/imgui/backends/imgui_impl_sdl.cpp"
"lib/imgui/backends/imgui_impl_opengl3.cpp"
"lib/imgui-filebrowser/*.cpp"
"lib/imgui-filebrowser/dirent/*.cpp"
"lib/ImGuizmo/*.cpp"
"lib/hash-library/md5.cpp"
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
    set_target_properties(prt3 PROPERTIES LINK_FLAGS "${CMAKE_LINK_FLAGS} -std=c++17 -MMD -MP -Oz -s USE_SDL=2 --use-preload-plugins -s USE_SDL_IMAGE=2 -s WASM=1 -sLLD_REPORT_UNDEFINED -sASSERTIONS --pre-js settings.js --embed-file assets --embed-file run.py -sALLOW_MEMORY_GROWTH -s EXCEPTION_DEBUG -sNO_DISABLE_EXCEPTION_CATCHING -s SDL2_IMAGE_FORMATS='[\"bmp\",\"png\"]' -sFULL_ES2 -sFULL_ES3 -s TOTAL_MEMORY=323813376 -sERROR_ON_WASM_CHANGES_AFTER_LINK -sWASM_BIGINT -O1")
endif ()

# Set compiler flags
target_compile_options(prt3 PUBLIC -Wall -Wextra -o2 -g -fsanitize=address -fno-omit-frame-pointer)
target_link_options(prt3 PUBLIC -Wall -Wextra -o2 -g -fsanitize=address -fno-omit-frame-pointer)
# Link libraries
target_link_libraries(prt3 assimp::assimp)
target_link_libraries(prt3 glm)
target_link_libraries(prt3 SDL2)
