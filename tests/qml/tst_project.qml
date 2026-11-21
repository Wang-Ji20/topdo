import "../../plasmoid/package/contents/ui"
import QtQuick
import QtTest
import io.github.wangji20.topdo
import topdo.test

// The project column, driven with keys through the whole panel, on a real todo file.
// See docs/design/project-today.md.
Item {
    id: root

    // Milestones of AidWalk: 1 Paper (steps 2 done, 3), 4 Data (done), 5 Supplement.
    readonly property string file: "Paper +AidWalk id:1\nx 2026-11-20 Run baselines p:1\nWrite discussion p:1\nx 2026-11-18 Data +AidWalk\nSupplement +AidWalk\nReport +Work\n"
    property string path

    width: 1400
    height: 900

    TodoModel {
        id: todoModel

        path: "/nonexistent/topdo-test/todo.txt"
    }

    ProjectModel {
        id: projectModel

        todos: todoModel
    }

    // A day without a calendar: what TodayModel offers, as plain values.
    QtObject {
        id: fakeDay

        property var events: []
        property date now: new Date()
        property int next: -1
        property string countdown: ""
    }

    Panel {
        id: panel

        width: implicitWidth
        height: implicitHeight
        todos: todoModel
        project: projectModel
        day: fakeDay
        fullHeight: 800
        onProjectPicked: name => {
            return projectModel.project = name;
        }
    }

    TestCase {
        function init() {
            root.path = Fixture.write(root.file);
            todoModel.path = root.path;
            projectModel.project = "";
            panel.open(-1);
            panel.forceActiveFocus();
        }

        function column(): Item {
            return findChild(panel, "projectColumn");
        }

        function test_overviewShowsTheCurrentMilestone() {
            compare(projectModel.name, "AidWalk");
            const shown = column();
            compare(shown.currentMilestone.title, "Paper +AidWalk");
            compare(shown.nextStep.title, "Write discussion");
        }

        function test_enterTicksTheHighlightedRow() {
            keyClick(Qt.Key_2);
            compare(column().rows.length, 5, "three milestones and the current one's two steps");
            keyClick(Qt.Key_Down);
            keyClick(Qt.Key_Down); // Write discussion
            keyClick(Qt.Key_Return);
            verify(/\nx \d{4}-\d{2}-\d{2} Write discussion p:1\n/.test(Fixture.read(root.path)), Fixture.read(root.path));
        }

        function test_pageDownPicksTheNextProject() {
            keyClick(Qt.Key_2);
            keyClick(Qt.Key_PageDown);
            compare(projectModel.name, "Work");
            keyClick(Qt.Key_PageDown);
            compare(projectModel.name, "AidWalk", "wraps around");
            keyClick(Qt.Key_PageUp);
            compare(projectModel.name, "Work");
        }

        function test_escapeGoesBack() {
            keyClick(Qt.Key_2);
            compare(panel.openColumn, 1);
            keyClick(Qt.Key_Escape);
            compare(panel.openColumn, -1);
        }

        name: "Project"
        when: windowShown
    }
}
