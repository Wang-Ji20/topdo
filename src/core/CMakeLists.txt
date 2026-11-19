# Pure functions on value types: no QObject, no I/O. See docs/architecture.md.
add_library(topdocore STATIC
    document.cpp
    document.h
    todos.cpp
    todos.h
)
target_include_directories(topdocore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(topdocore PUBLIC Qt6::Core)
