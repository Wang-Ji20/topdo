# The todo file on disk and the config file. See docs/architecture.md.
add_library(topdostore STATIC
    store.cpp
    store.h
)
target_include_directories(topdostore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(topdostore PUBLIC topdocore Qt6::Core)
