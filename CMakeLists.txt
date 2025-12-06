cmake_minimum_required(VERSION 3.10)
project(mydesertcolony)

set(CMAKE_CXX_STANDARD 11)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
    execute_process(
        COMMAND xcrun --show-sdk-path
        OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Using macOS SDK: ${CMAKE_OSX_SYSROOT}")
endif()

find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(assimp CONFIG REQUIRED)

if(TARGET assimp::assimp)
  get_target_property(_ali assimp::assimp INTERFACE_LINK_LIBRARIES)
  message(STATUS "assimp::assimp INTERFACE_LINK_LIBRARIES = ${_ali}")
endif()

# TONOTE: This is a hack to allow code to run
set(_BAD_Z "/Library/Developer/CommandLineTools/SDKs/MacOSX26.sdk/usr/lib/libz.tbd")
if(TARGET assimp::assimp)
  get_target_property(_ali assimp::assimp INTERFACE_LINK_LIBRARIES)
  if(_ali MATCHES "${_BAD_Z}")
    string(REPLACE "${_BAD_Z}" "" _ali "${_ali}")
    set_target_properties(assimp::assimp PROPERTIES INTERFACE_LINK_LIBRARIES "${_ali}")
    message(STATUS "Patched assimp::assimp INTERFACE_LINK_LIBRARIES to remove bad zlib path")
  endif()
endif()

list(APPEND _EXTRA_Z_LIB z)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# use external glfw/glad/glm & headers

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/glfw-3.1.2")
set(GLAD_ROOT "${CMAKE_CURRENT_LIST_DIR}/external/glad-3.3")
set(GLAD_INC  "${GLAD_ROOT}/include")

if (EXISTS "${GLAD_ROOT}/src/glad.c" AND EXISTS "${GLAD_INC}/glad/glad.h")
    message(STATUS "GLAD: using GLAD1 (glad/glad.h + src/glad.c)")
    add_library(glad "${GLAD_ROOT}/src/glad.c")
    target_include_directories(glad PUBLIC "${GLAD_INC}")
elseif (EXISTS "${GLAD_ROOT}/src/gl.c" AND EXISTS "${GLAD_INC}/glad/gl.h")
    message(STATUS "GLAD: using GLAD2 (glad/gl.h + src/gl.c)")
    add_library(glad "${GLAD_ROOT}/src/gl.c")
    target_include_directories(glad PUBLIC "${GLAD_INC}")
else()
    message(FATAL_ERROR "GLAD sources not found in ${GLAD_ROOT}")
endif()

set(PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/code/include")
include_directories("${PROJECT_INCLUDE_DIR}")
include_directories("${CMAKE_CURRENT_LIST_DIR}/external/glm-0.9.7.1")

add_executable(mydesertcolony_main
    code/src/main.cpp
    code/src/callbacks.cpp
    code/src/renderer.cpp
    code/src/shader_utils.cpp
    code/src/texture_loader.cpp
    code/src/render_utils/animate.cpp
    code/src/render_utils/hierarchy_utils.cpp
    code/src/render_utils/mesh_loader.cpp
    code/src/render_utils/model_render.cpp
    code/src/render_utils/scene_manager.cpp
    code/src/render_utils/shader_uniform.cpp
    code/src/render_utils/transform_utils.cpp
    code/src/render_utils/animator.cpp
    code/src/env_manager/skybox.cpp
    code/src/env_manager/terrain_manager.cpp
    code/src/env_manager/terrain_loader.cpp
)

target_compile_definitions(mydesertcolony_main PRIVATE GL_SILENCE_DEPRECATION)
target_include_directories(mydesertcolony_main PRIVATE
    "${PROJECT_INCLUDE_DIR}"
    "${GLAD_INC}"
    "${CMAKE_CURRENT_LIST_DIR}/external/assimp/contrib/stb"
)

target_link_libraries(mydesertcolony_main PRIVATE
  glad
  glfw
  ${OPENGL_LIBRARIES}
  assimp::assimp
  ${ZLIB_LIBRARIES}
  ${_EXTRA_Z_LIB}
)

if(APPLE)
    target_link_libraries(mydesertcolony_main PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()

add_custom_command(TARGET mydesertcolony_main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/code/assets/shaders/desert.frag"
        "${CMAKE_SOURCE_DIR}/code/assets/shaders/desert.vert"
        "${CMAKE_SOURCE_DIR}/code/assets/shaders/terrain.frag"
        "${CMAKE_SOURCE_DIR}/code/assets/shaders/terrain.vert"
        "${CMAKE_SOURCE_DIR}/code/assets/shaders/skybox.vert"
        "${CMAKE_SOURCE_DIR}/code/assets/shaders/skybox.frag"
        $<TARGET_FILE_DIR:mydesertcolony_main>
    COMMENT "Copying shader files to output folder"
)

add_custom_command(TARGET mydesertcolony_main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:mydesertcolony_main>/assets/models"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/code/assets/models"
        "$<TARGET_FILE_DIR:mydesertcolony_main>/assets/models"
    COMMENT "Copying models folder to output folder"
)

add_custom_command(TARGET mydesertcolony_main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/code/assets/textures"
        "$<TARGET_FILE_DIR:mydesertcolony_main>/textures"
    COMMENT "Copying textures directory to output folder"
)

add_custom_command(TARGET mydesertcolony_main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/code/assets/shaders"
        "$<TARGET_FILE_DIR:mydesertcolony_main>/shaders"
    COMMENT "Copying shaders directory to output folder"
)

# after find_package(ZLIB REQUIRED)
# If ZLIB_LIBRARIES/ ZLIB_LIBRARY points to a missing .tbd, fall back to linking "z"
if (DEFINED ZLIB_LIBRARIES)
  set(_zcheck "${ZLIB_LIBRARIES}")
elseif (DEFINED ZLIB_LIBRARY)
  set(_zcheck "${ZLIB_LIBRARY}")
else()
  set(_zcheck "")
endif()

if(NOT _zcheck STREQUAL "" AND NOT EXISTS "${_zcheck}")
  message(WARNING "ZLIB found at ${_zcheck} but file missing — falling back to link 'z'")
  set(ZLIB_LIBRARIES z)
  set(ZLIB_LIBRARY z)
endif()
