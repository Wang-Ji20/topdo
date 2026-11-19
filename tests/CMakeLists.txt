# Generator of random todo files for property tests; see AGENTS.md, "Testing".
add_library(topdotestsupport STATIC
    support/randomfile.cpp
    support/randomfile.h
)
target_include_directories(topdotestsupport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/support)
target_link_libraries(topdotestsupport PUBLIC Qt6::Core)

# A QTest executable registered with CTest. Golden files are read from tests/data/.
function(topdo_add_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE topdocore topdostore topdocli topdotestsupport Qt6::Test)
    target_compile_definitions(${name} PRIVATE TOPDO_TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/data")
    add_test(NAME ${name} COMMAND ${name})
endfunction()

topdo_add_test(tst_document core/tst_document.cpp)
topdo_add_test(tst_todos core/tst_todos.cpp)
topdo_add_test(tst_edits core/tst_edits.cpp)
topdo_add_test(tst_store store/tst_store.cpp)
topdo_add_test(tst_cli cli/tst_cli.cpp)
topdo_add_test(tst_query core/tst_query.cpp)
