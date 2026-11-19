cmake_minimum_required(VERSION 3.25)
project(topdo VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # for clangd and clang-tidy

option(TOPDO_WERROR "Treat compiler warnings as errors" OFF)
add_compile_options(-Wall -Wextra -Wpedantic)
if(TOPDO_WERROR)
    add_compile_options(-Werror)
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Test)
qt_standard_project_setup()

enable_testing()

add_subdirectory(src/core)
add_subdirectory(src/store)
add_subdirectory(src/cli)
add_subdirectory(tests)
