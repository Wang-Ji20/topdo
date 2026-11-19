# The QML module the plasmoid imports: io.github.wangji20.topdo. See docs/architecture.md.
qt_add_qml_module(topdoqml
    URI io.github.wangji20.topdo
    VERSION 1.0
    SOURCES
        windoweffects.cpp
        windoweffects.h
)
target_link_libraries(topdoqml PRIVATE Qt6::Quick KF6::WindowSystem)
