import QtQuick
import QtQuick.Window
import io.github.wangji20.topdo
import "package/contents/ui"

// Shows the panel in a plain window, without Plasma, on the todo file given as the last argument;
// --open=N opens column N (0, 1 or 2) at start:
//   QML_IMPORT_PATH=build/dev/qml /usr/lib64/qt6/bin/qml plasmoid/preview.qml -- [--open=0] todo.txt
// There is no blur here; the background shapes only show that the glass is translucent.
Window {
    width: panel.implicitWidth + 80
    height: panel.height + 80
    visible: true
    color: "#15161a"
    title: "topdo preview"

    Rectangle {
        x: 60
        y: 120
        width: 360
        height: 360
        radius: 180
        color: "#3b4252"
    }

    Rectangle {
        x: parent.width - 420
        y: 30
        width: 300
        height: 300
        rotation: 12
        color: "#2e3440"
    }

    TodoModel {
        id: todoModel

        path: Qt.application.arguments[Qt.application.arguments.length - 1]
    }

    ProjectModel {
        id: projectModel

        todos: todoModel
    }

    Panel {
        id: panel

        x: 40
        y: 40
        width: implicitWidth
        height: implicitHeight
        todos: todoModel
        project: projectModel
        fullHeight: 640
        onProjectPicked: name => {
            return projectModel.project = name;
        }
        onCloseRequested: Qt.quit()
        Component.onCompleted: {
            const open = Qt.application.arguments.find(argument => {
                return argument.startsWith("--open=");
            });
            if (open)
                panel.open(Number(open.slice("--open=".length)));
        }
    }
}
