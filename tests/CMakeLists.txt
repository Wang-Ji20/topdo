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

add_executable(tst_day calendar/tst_day.cpp)
target_link_libraries(tst_day PRIVATE topdoday Qt6::Test)
add_test(NAME tst_day COMMAND tst_day)

# The QML module's C++ types, tested without QML.
add_executable(tst_todomodel qml/tst_todomodel.cpp)
target_link_libraries(tst_todomodel PRIVATE topdoqml Qt6::Test)
add_test(NAME tst_todomodel COMMAND tst_todomodel)

# The plasmoid's QML, driven by real key presses (Qt Quick Test), offscreen, with a fixture for
# todo files; the tests are tests/qml/tst_*.qml.
find_package(Qt6 REQUIRED COMPONENTS QuickTest)
add_executable(tst_quick qml/tst_quick.cpp)
target_link_libraries(tst_quick PRIVATE Qt6::QuickTest Qt6::Qml)
target_compile_definitions(tst_quick PRIVATE
    QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml"
    TOPDO_QML_DIR="${QT_QML_OUTPUT_DIRECTORY}")
add_dependencies(tst_quick topdoqml)
add_test(NAME tst_quick COMMAND tst_quick)
set_tests_properties(tst_quick PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
