cmake_minimum_required(VERSION 4.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

project(RadianceCascades CXX)

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

# Declaring our executable
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} glm)

## Optimization and safety flags
target_compile_options(${PROJECT_NAME} PRIVATE
    -O3
    -fno-rtti
    -fstrict-aliasing
    -ffast-math
    -fomit-frame-pointer
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wdouble-promotion
    -Wold-style-cast
    -Wnon-virtual-dtor
    -Woverloaded-virtual
)

add_compile_definitions(GLFW_INCLUDE_NONE)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)
