import "../../plasmoid/package/contents/ui"
import QtQuick
import QtTest
import io.github.wangji20.topdo

// The panel's keys in the overview and with a column open; see docs/design/widget.md, "Panel".
Item {
    width: 1400
    height: 900

    TodoModel {
        id: todoModel

        path: "/nonexistent/topdo-test/todo.txt" // a missing file reads as an empty list
    }

    ProjectModel {
        id: projectModel

        todos: todoModel
    }

    Panel {
        id: panel

        width: implicitWidth
        height: implicitHeight
        todos: todoModel
        project: projectModel
        fullHeight: 800
        onProjectPicked: name => {
            return projectModel.project = name;
        }
    }

    SignalSpy {
        id: closed

        target: panel
        signalName: "closeRequested"
    }

    TestCase {
        function init() {
            panel.openColumn = -1;
            panel.focusColumn = 0;
            panel.forceActiveFocus();
            closed.clear();
        }

        function test_arrowsAndTabPickColumns() {
            keyClick(Qt.Key_Right);
            compare(panel.focusColumn, 1);
            keyClick(Qt.Key_Tab);
            compare(panel.focusColumn, 2);
            keyClick(Qt.Key_Right);
            compare(panel.focusColumn, 0, "wraps around");
            keyClick(Qt.Key_Left);
            compare(panel.focusColumn, 2);
            keyClick(Qt.Key_Backtab);
            compare(panel.focusColumn, 1);
            compare(panel.openColumn, -1, "picking doesn't open");
        }

        function test_enterOpensThePickedColumn() {
            keyClick(Qt.Key_Right);
            keyClick(Qt.Key_Return);
            compare(panel.openColumn, 1);
        }

        function test_numbersOpenColumns() {
            keyClick(Qt.Key_3);
            compare(panel.openColumn, 2);
            compare(panel.focusColumn, 2);
            keyClick(Qt.Key_1);
            compare(panel.openColumn, 0);
        }

        function test_arrowsMoveTheOpenColumn() {
            keyClick(Qt.Key_1);
            keyClick(Qt.Key_Left);
            compare(panel.openColumn, 2);
            keyClick(Qt.Key_Tab);
            compare(panel.openColumn, 0);
        }

        function test_escapeStepsBack() {
            keyClick(Qt.Key_2);
            keyClick(Qt.Key_Escape);
            compare(panel.openColumn, -1);
            compare(closed.count, 0);
            keyClick(Qt.Key_Escape);
            compare(closed.count, 1);
        }

        function test_anOpenColumnTakesTheFullHeight() {
            verify(panel.implicitHeight < panel.fullHeight);
            keyClick(Qt.Key_1);
            tryCompare(panel, "implicitHeight", panel.fullHeight);
        }

        name: "Panel"
        when: windowShown
    }
}
