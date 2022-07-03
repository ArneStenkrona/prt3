# Install script for directory: /Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/Users/arnestenkrona/Documents/repositories/emsdk/upstream/emscripten/cache/sysroot")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibassimp5.2.0-devx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/lib/libassimp.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xassimp-devx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp" TYPE FILE FILES
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/anim.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/aabb.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/ai_assert.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/camera.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/color4.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/color4.inl"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/config.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/ColladaMetaData.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/commonMetaData.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/defs.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/cfileio.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/light.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/material.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/material.inl"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/matrix3x3.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/matrix3x3.inl"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/matrix4x4.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/matrix4x4.inl"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/mesh.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/ObjMaterial.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/pbrmaterial.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/GltfMaterial.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/postprocess.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/quaternion.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/quaternion.inl"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/scene.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/metadata.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/texture.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/types.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/vector2.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/vector2.inl"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/vector3.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/vector3.inl"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/version.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/cimport.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/importerdesc.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/Importer.hpp"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/DefaultLogger.hpp"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/ProgressHandler.hpp"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/IOStream.hpp"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/IOSystem.hpp"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/Logger.hpp"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/LogStream.hpp"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/NullLogger.hpp"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/cexport.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/Exporter.hpp"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/DefaultIOStream.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/DefaultIOSystem.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/ZipArchiveIOSystem.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/SceneCombiner.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/fast_atof.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/qnan.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/BaseImporter.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/Hash.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/MemoryIOWrapper.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/ParsingUtils.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/StreamReader.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/StreamWriter.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/StringComparison.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/StringUtils.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/SGSpatialSort.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/GenericProperty.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/SpatialSort.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/SkeletonMeshBuilder.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/SmallVector.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/SmoothingGroups.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/SmoothingGroups.inl"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/StandardShapes.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/RemoveComments.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/Subdivision.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/Vertex.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/LineSplitter.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/TinyFormatter.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/Profiler.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/LogAux.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/Bitmap.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/XMLTools.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/IOStreamBuffer.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/CreateAnimMesh.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/XmlParser.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/BlobIOSystem.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/MathFunctions.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/Exceptional.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/ByteSwapper.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/Base64.hpp"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xassimp-devx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp/Compiler" TYPE FILE FILES
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/Compiler/pushpack1.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/Compiler/poppack1.h"
    "/Users/arnestenkrona/Documents/repositories/prt3/lib/assimp/code/../include/assimp/Compiler/pstdint.h"
    )
endif()

