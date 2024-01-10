cmake_minimum_required (VERSION 3.14.4)
project(prt3)

# Set C++ language version to C++17
set(CMAKE_CXX_STANDARD 17)

# Set relase/debug
set(CMAKE_BUILD_TYPE release)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Remove previously built .js-file
# (necessary to force regeneration of assets)
file(REMOVE "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.js")

# Set paths
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories(
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/lib/imgui"
  "${PROJECT_SOURCE_DIR}/lib/imgui-filebrowser"
  "${PROJECT_SOURCE_DIR}/lib/ImGuizmo"
  "${PROJECT_SOURCE_DIR}/lib/hash-library"
  "${PROJECT_SOURCE_DIR}/lib/stb_image"
  "${PROJECT_SOURCE_DIR}/lib/minivorbis"
  "${PROJECT_SOURCE_DIR}/lib/o1heap"
)

file(GLOB SOURCES
  "src/main/main.cpp"
  "src/backend/dummy/dummy_renderer.cpp"
  "src/backend/opengl/gl_material_manager.cpp"
  "src/backend/opengl/gl_material.cpp"
  "src/backend/opengl/gl_mesh.cpp"
  "src/backend/opengl/gl_model_manager.cpp"
  "src/backend/opengl/gl_postprocessing_chain.cpp"
  "src/backend/opengl/gl_postprocessing_pass.cpp"
  "src/backend/opengl/gl_renderer.cpp"
  "src/backend/opengl/gl_texture_manager.cpp"
  "src/backend/opengl/gl_shader_utility.cpp"
  "src/backend/opengl/gl_shader.cpp"
  "src/backend/opengl/gl_source_buffers.cpp"
  "src/backend/opengl/gl_utility.cpp"
  "src/daedalus/character/character_controller.cpp"
  "src/daedalus/character/player_controller.cpp"
  "src/daedalus/game_state/game_state.cpp"
  "src/daedalus/game_state/item_db.cpp"
  "src/daedalus/game_state/object_db.cpp"
  "src/daedalus/game_state/prefab_db.cpp"
  "src/daedalus/game_state/sound_pool.cpp"
  "src/daedalus/gui/game_gui.cpp"
  "src/daedalus/gui/strings.cpp"
  "src/daedalus/map/map.cpp"
  "src/daedalus/npc/npc_action.cpp"
  "src/daedalus/npc/npc_controller.cpp"
  "src/daedalus/npc/npc_db.cpp"
  "src/daedalus/npc/npcs.cpp"
  "src/daedalus/objects/interactable.cpp"
  "src/daedalus/objects/slide.cpp"
  "src/engine/animation/animation_system.cpp"
  "src/engine/audio/audio_manager.cpp"
  "src/engine/audio/audio_mem_util.cpp"
  "src/engine/audio/include_vorbis.c"
  "src/engine/component/canvas/canvas.cpp"
  "src/engine/component/script/script.cpp"
  "src/engine/component/animated_mesh.cpp"
  "src/engine/component/animated_model.cpp"
  "src/engine/component/armature.cpp"
  "src/engine/component/collider_component.cpp"
  "src/engine/component/component_manager.cpp"
  "src/engine/component/component_utility.cpp"
  "src/engine/component/decal.cpp"
  "src/engine/component/door.cpp"
  "src/engine/component/material.cpp"
  "src/engine/component/mesh.cpp"
  "src/engine/component/model.cpp"
  "src/engine/component/navigation_mesh.cpp"
  "src/engine/component/particle_system.cpp"
  "src/engine/component/point_light.cpp"
  "src/engine/component/script_set.cpp"
  "src/engine/component/sound_source.cpp"
  "src/engine/component/weapon.cpp"
  "src/engine/physics/aabb_tree.cpp"
  "src/engine/physics/aabb.cpp"
  "src/engine/physics/collider.cpp"
  "src/engine/physics/gjk.cpp"
  "src/engine/physics/physics_system.cpp"
  "src/engine/project/project.cpp"
  "src/engine/core/context.cpp"
  "src/engine/core/engine.cpp"
  "src/engine/core/input.cpp"
  "src/engine/editor/action/action_add_node.cpp"
  "src/engine/editor/action/action_instantiate_prefab.cpp"
  "src/engine/editor/action/action_manager.cpp"
  "src/engine/editor/action/action_remove_node.cpp"
  "src/engine/editor/action/action_transform_node.cpp"
  "src/engine/editor/gizmo/gizmo_manager.cpp"
  "src/engine/editor/gui_components/component/component_gui_utility.cpp"
  "src/engine/editor/gui_components/editor_gui.cpp"
  "src/engine/editor/gui_components/menu.cpp"
  "src/engine/editor/gui_components/node_inspector.cpp"
  "src/engine/editor/gui_components/panel.cpp"
  "src/engine/editor/gui_components/project_config.cpp"
  "src/engine/editor/gui_components/scene_hierarchy.cpp"
  "src/engine/editor/gui_components/scene_inspector.cpp"
  "src/engine/editor/editor_camera.cpp"
  "src/engine/editor/editor_context.cpp"
  "src/engine/editor/editor.cpp"
  "src/engine/navigation/navigation_system.cpp"
  "src/engine/geometry/shapes.cpp"
  "src/engine/rendering/camera.cpp"
  "src/engine/rendering/material_manager.cpp"
  "src/engine/rendering/model_manager.cpp"
  "src/engine/rendering/model.cpp"
  "src/engine/rendering/renderer.cpp"
  "src/engine/rendering/texture_manager.cpp"
  "src/engine/rendering/texture.cpp"
  "src/engine/scene/node.cpp"
  "src/engine/scene/prefab.cpp"
  "src/engine/scene/scene_manager.cpp"
  "src/engine/scene/scene.cpp"
  "src/engine/scene/script_container.cpp"
  "src/engine/scene/transform_cache.cpp"
  "src/util/checksum.cpp"
  "src/util/file_util.cpp"
  "src/util/geometry_util.cpp"
  "src/util/mesh_util.cpp"
  # libs
  "lib/imgui/*.cpp"
  "lib/imgui/backends/imgui_impl_glfw.cpp"
  "lib/imgui/backends/imgui_impl_opengl3.cpp"
  "lib/imgui-filebrowser/*.cpp"
  "lib/imgui-filebrowser/dirent/*.cpp"
  "lib/ImGuizmo/*.cpp"
  "lib/hash-library/md5.cpp"
  "lib/hash-library/crc32.cpp"
  "lib/o1heap/o1heap.c"
)

add_subdirectory(lib/glm EXCLUDE_FROM_ALL)

# Add the executable
add_executable(prt3 ${SOURCES})

target_include_directories(prt3 SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/lib/tsf")

include_directories("${OPENAL_INCLUDE_DIR}/AL")

add_custom_target(package_resources
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/
  COMMAND $ENV{EMSDK}/upstream/emscripten/tools/file_packager prt3.data --preload assets/ --use-preload-plugins --js-output=data.js
)
add_dependencies(prt3 package_resources)

# Wasm specific
if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
  set(CMAKE_CXX_FLAGS "-s USE_ZLIB=1")
  set_target_properties(prt3 PROPERTIES LINK_FLAGS "${CMAKE_LINK_FLAGS} -std=c++17 --use-preload-plugins -s WASM=1 -sASSERTIONS=1 --pre-js settings.js -s DISABLE_EXCEPTION_CATCHING=1 -s MIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sGL_PREINITIALIZED_CONTEXT=1 -s TOTAL_MEMORY=323813376 -sALLOW_MEMORY_GROWTH -sWASM_BIGINT -sERROR_ON_WASM_CHANGES_AFTER_LINK -O1 -sFORCE-FILESYSTEM -lopenal -s USE_ZLIB=1 -sUSE_GLFW=3 -sSTACK_SIZE=5MB")
endif ()

set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_FBX_IMPORTER ON)
add_subdirectory(lib/assimp EXCLUDE_FROM_ALL)

# Set compiler flags
target_compile_options(prt3 PUBLIC -Wall -Wextra -o2 -g -fno-omit-frame-pointer)
target_link_options(prt3 PUBLIC -Wall -Wextra -o2 -g -fno-omit-frame-pointer)
# Link libraries
target_link_libraries(prt3 PRIVATE assimp::assimp)
target_link_libraries(prt3 PRIVATE glm)
