cmake_minimum_required(VERSION 3.25)
project(topdo VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # for clangd and clang-tidy

option(TOPDO_WERROR "Treat compiler warnings as errors" OFF)
add_compile_options(-Wall -Wextra -Wpedantic)
if(TOPDO_WERROR)
    add_compile_options(-Werror)
endif()

enable_testing()

# Qt, KDE Frameworks and the targets under src/ arrive with the first code;
# see PLAN.md.
