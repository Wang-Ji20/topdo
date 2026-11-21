# The QML module the plasmoid imports: io.github.wangji20.topdo. See docs/architecture.md.
qt_add_qml_module(topdoqml
    URI io.github.wangji20.topdo
    VERSION 1.0
    SOURCES
        projectmodel.cpp
        projectmodel.h
        todomodel.cpp
        todomodel.h
        windoweffects.cpp
        windoweffects.h
)
target_link_libraries(topdoqml PUBLIC topdostore Qt6::Quick PRIVATE KF6::WindowSystem)
target_include_directories(topdoqml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Installed in the layout of Qt's own modules: <prefix>/lib64/qt6/qml/io/github/wangji20/topdo/.
# The module's library goes into the same folder, where the plugin finds it ($ORIGIN).
set(TOPDO_QML_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/qt6/qml/io/github/wangji20/topdo)
set_target_properties(topdoqmlplugin PROPERTIES INSTALL_RPATH "$ORIGIN")
install(TARGETS topdoqml topdoqmlplugin
    LIBRARY DESTINATION ${TOPDO_QML_INSTALL_DIR}
    NAMELINK_SKIP)
install(FILES
    ${QT_QML_OUTPUT_DIRECTORY}/io/github/wangji20/topdo/qmldir
    ${QT_QML_OUTPUT_DIRECTORY}/io/github/wangji20/topdo/topdoqml.qmltypes
    DESTINATION ${TOPDO_QML_INSTALL_DIR})
