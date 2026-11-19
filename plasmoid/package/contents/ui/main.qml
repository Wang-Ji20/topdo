pragma ComponentBehavior: Bound

// Experiment: a badge that opens our own frameless panel, with KWin's blur behind it.
// See docs/design/widget.md, "Our own frame".
import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import io.github.wangji20.topdo

PlasmoidItem {
    id: root

    readonly property real slabRadius: Kirigami.Units.cornerRadius * 3

    preferredRepresentation: compactRepresentation

    compactRepresentation: Item {
        Layout.preferredWidth: badge.implicitWidth
        Layout.preferredHeight: badge.implicitHeight

        Rectangle {
            id: badge

            anchors.centerIn: parent
            implicitWidth: badgeText.implicitWidth + Kirigami.Units.gridUnit
            implicitHeight: badgeText.implicitHeight + Kirigami.Units.smallSpacing * 2
            radius: height / 2
            color: Qt.rgba(1, 1, 1, 0.06)
            border.color: Qt.rgba(1, 1, 1, 0.3)

            Text {
                id: badgeText

                anchors.centerIn: parent
                text: "● topdo"
                color: Kirigami.Theme.textColor
            }

            MouseArea {
                anchors.fill: parent
                onClicked: panel.visible = !panel.visible
            }
        }
    }

    // Experiment: open the panel shortly after start, to look at it without clicking.
    Timer {
        interval: 1500
        running: true
        onTriggered: panel.visible = true
    }

    PlasmaCore.Dialog {
        id: panel

        visualParent: root.compactRepresentationItem
        location: PlasmaCore.Types.Floating
        type: PlasmaCore.Dialog.PopupMenu
        backgroundHints: PlasmaCore.Dialog.NoBackground
        hideOnWindowDeactivate: true
        onVisibleChanged: blur()
        onWidthChanged: blur()
        onHeightChanged: blur()

        function blur() {
            if (visible) {
                WindowEffects.blurBehind(panel, Qt.rect(0, 0, slab.width, slab.height), root.slabRadius);
            }
        }

        mainItem: Rectangle {
            id: slab

            width: Kirigami.Units.gridUnit * 56
            height: Kirigami.Units.gridUnit * 22
            radius: root.slabRadius
            color: Qt.rgba(42 / 255, 44 / 255, 51 / 255, 0.88)
            border.color: Qt.rgba(1, 1, 1, 0.3)

            Text {
                anchors.centerIn: parent
                text: "glass slab"
                color: Kirigami.Theme.textColor
                font.pixelSize: Kirigami.Units.gridUnit * 3
            }
        }
    }
}
