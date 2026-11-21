import "../../plasmoid/package/contents/ui"
import QtQuick
import QtQuick.Layouts
import QtTest
import io.github.wangji20.topdo

// The badge's pill stays at the bar's centre while the widgets beside it change width;
// see docs/design/widget.md, "How it is built". The bar is a row like Plasma's: each widget
// sits in a container that the row moves and resizes.
Item {
    id: screen

    width: 1200
    height: 100

    TodoModel {
        id: todoModel

        path: "/nonexistent/topdo-test/todo.md" // a missing file reads as an empty list
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: appMenu

            implicitWidth: 200
            Layout.fillHeight: true
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: screen.width

            Badge {
                id: badge

                anchors.fill: parent
                todos: todoModel
            }

        }

        Rectangle {
            id: tray

            implicitWidth: 150
            Layout.fillHeight: true
        }

    }

    TestCase {
        function pillCentre() {
            const pill = findChild(badge, "pill");
            return pill.mapToItem(screen, pill.width / 2, 0).x;
        }

        function test_centredAtStart() {
            tryVerify(() => {
                return Math.abs(pillCentre() - screen.width / 2) < 1;
            }, 1000, `centre at ${pillCentre()}`);
        }

        function test_staysCentredWhenTheAppMenuShrinks() {
            appMenu.implicitWidth = 50;
            tryVerify(() => {
                return Math.abs(pillCentre() - screen.width / 2) < 1;
            }, 1000, `centre at ${pillCentre()}`);
            appMenu.implicitWidth = 300;
            tryVerify(() => {
                return Math.abs(pillCentre() - screen.width / 2) < 1;
            }, 1000, `centre at ${pillCentre()}`);
        }

        name: "Badge"
        when: windowShown
    }

}
