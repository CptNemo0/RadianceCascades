cmake_minimum_required(VERSION 4.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

# The project includes headers the classic way (no `import`); disable C++20 module
# scanning so CMake stops emitting GCC-only flags (-fmodules-ts, -fmodule-mapper,
# -fdeps-format) that clangd cannot parse. Also speeds up the build.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

project(RadianceCascades CXX)

# Fall back to an optimized build when configured without a preset / build type.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG        7b6aead9fb88b3623e3b3725ebb42670cbe4c579
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG        8d1fd52e5ab5590e2c81768ace50c72bae28f2ed
)
FetchContent_MakeAvailable(glm)

# Adding our source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/sources/*.cc" "${CMAKE_CURRENT_LIST_DIR}/sources/*.cpp") # Define PROJECT_SOURCES as a list of all source files
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/sources/" "${CMAKE_CURRENT_LIST_DIR}/sources/imgui" "${CMAKE_CURRENT_LIST_DIR}/sources/imgui/backends")

set_source_files_properties(sources/glad/src/glad.cc PROPERTIES COMPILE_OPTIONS "-w")

# Declaring our executable
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} glm)

# C++23 std::print's Unicode terminal output lives in libstdc++exp on GCC/MinGW
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(${PROJECT_NAME} stdc++exp)
endif()

## Warnings (all configurations)
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wdouble-promotion
    -Wold-style-cast
    -Wnon-virtual-dtor
    -Woverloaded-virtual
)

## Optimization comes from the build type: Release => -O3 -DNDEBUG, Debug => -O0 -g.
## MinGW GCC ships no ASan/UBSan, so Debug enables the runtime checks it *does*
## have: libstdc++ assertions (bounds/precondition checks on containers and
## iterators), stack canaries, and frame pointers for clean stack traces.
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
    $<$<CONFIG:Debug>:-fstack-protector-strong>
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>
)

add_compile_definitions(GLFW_INCLUDE_NONE)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)
