# The QML module the plasmoid imports: io.github.wangji20.topdo. See docs/architecture.md.
qt_add_qml_module(topdoqml
    URI io.github.wangji20.topdo
    VERSION 1.0
    SOURCES
        todomodel.cpp
        todomodel.h
        windoweffects.cpp
        windoweffects.h
)
target_link_libraries(topdoqml PUBLIC topdostore Qt6::Quick PRIVATE KF6::WindowSystem)
target_include_directories(topdoqml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
