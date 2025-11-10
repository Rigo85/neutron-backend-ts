cmake_minimum_required(VERSION 3.20)
project(native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif ()

# Warnings y optimizaciones según build type
if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-Wall -Wextra -Wpedantic -O0 -g)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif ()

option(ENABLE_ASAN "Enable AddressSanitizer" ON)
if (ENABLE_ASAN AND CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif ()

set(ENGINE_SOURCES
  src/Move.cpp
  src/FullMove.cpp
  src/gameutils.cpp
  src/Board.cpp
  src/minimax.cpp
  src/cleaners.cpp
  main.cpp
)

include_directories(include)

add_executable(native_debug ${ENGINE_SOURCES})
target_compile_definitions(native_debug PRIVATE ENGINE_STANDALONE=1)

