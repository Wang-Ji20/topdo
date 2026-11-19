pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.kirigami as Kirigami
import io.github.wangji20.topdo

// The badge: it takes the free space of the bar and draws a pill at the screen's exact centre,
// whatever the width of the app menu on its left and the tray on its right.
// See docs/design/widget.md, "Badge".
Item {
    id: badge

    required property TodoModel todos

    signal clicked

    // Places the pill at the centre of the screen, as far as the free space allows.
    function centre() {
        const screen = Plasmoid.containment.screenGeometry;
        const left = badge.mapToGlobal(0, 0).x - screen.x;
        const wanted = screen.width / 2 - left - pill.width / 2;
        pill.x = Math.max(0, Math.min(wanted, badge.width - pill.width));
    }

    function withAlpha(colour, alpha) {
        return Qt.rgba(colour.r, colour.g, colour.b, alpha);
    }

    // The app menu also stretches (it ends with a filler item), and the bar splits free space
    // between stretching widgets. Asking for the whole screen leaves the app menu its minimum,
    // which is exactly its buttons, and gives the badge everything else.
    Layout.fillWidth: true
    Layout.preferredWidth: Plasmoid.containment.screenGeometry.width
    Layout.fillHeight: true
    Layout.minimumWidth: Kirigami.Units.gridUnit * 6
    Component.onCompleted: centre()
    onWidthChanged: centre()

    Connections {
        function onScreenGeometryChanged() {
            badge.centre();
        }

        target: Plasmoid.containment
    }

    // The metal edge: a gradient that is light at the top and darker below, under the fill.
    Rectangle {
        id: pill

        anchors.verticalCenter: parent.verticalCenter
        width: Math.min(row.implicitWidth + Kirigami.Units.gridUnit, badge.width)
        height: Math.min(badge.height - Kirigami.Units.smallSpacing, row.implicitHeight + Kirigami.Units.smallSpacing * 2)
        radius: height / 2
        onWidthChanged: badge.centre()

        gradient: Gradient {
            GradientStop {
                position: 0
                color: badge.withAlpha(Kirigami.Theme.textColor, 0.42)
            }

            GradientStop {
                position: 0.55
                color: badge.withAlpha(Kirigami.Theme.textColor, 0.07)
            }

            GradientStop {
                position: 1
                color: badge.withAlpha(Kirigami.Theme.textColor, 0.2)
            }
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: height / 2
            color: badge.withAlpha(Qt.lighter(Kirigami.Theme.backgroundColor, 1.2), 0.92)
        }

        RowLayout {
            id: row

            anchors.centerIn: parent
            width: Math.min(implicitWidth, pill.width - Kirigami.Units.gridUnit)
            spacing: Kirigami.Units.smallSpacing * 2

            // A ring in the theme's highlight colour.
            Rectangle {
                Layout.alignment: Qt.AlignVCenter
                implicitWidth: Kirigami.Units.smallSpacing * 2
                implicitHeight: implicitWidth
                radius: width / 2
                color: "transparent"
                border.width: 2
                border.color: Kirigami.Theme.highlightColor
            }

            Text {
                Layout.fillWidth: true
                Layout.maximumWidth: Kirigami.Units.gridUnit * 16
                text: badge.todos.nextTitle || "all done"
                elide: Text.ElideRight
                color: Kirigami.Theme.textColor
                font.family: "Poppins"
            }

            Text {
                visible: badge.todos.openCount > 0
                text: `${badge.todos.openCount} open`
                color: badge.withAlpha(Kirigami.Theme.textColor, 0.7)
                font.family: "JetBrains Mono"
                font.pointSize: Kirigami.Theme.smallFont.pointSize
            }

            Text {
                visible: badge.todos.lateCount > 0
                text: `${badge.todos.lateCount} late`
                color: Kirigami.Theme.negativeTextColor
                font.family: "JetBrains Mono"
                font.pointSize: Kirigami.Theme.smallFont.pointSize
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: badge.clicked()
        }
    }
}
