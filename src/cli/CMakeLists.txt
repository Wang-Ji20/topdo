# The `topdo` command. The library holds everything but main(), so tests can call runCli().
add_library(topdocli STATIC
    cli.cpp
    cli.h
)
target_include_directories(topdocli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(topdocli PUBLIC topdostore Qt6::Core)

add_executable(topdo main.cpp)
target_link_libraries(topdo PRIVATE topdocli)
install(TARGETS topdo)
