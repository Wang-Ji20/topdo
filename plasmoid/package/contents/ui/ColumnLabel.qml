import QtQuick
import QtQuick.Layouts

// The head of a column: a square that fills when the column is picked, its number (which is
// also its key), its name, and a "stub" tag for columns without real data yet.
RowLayout {
    id: label

    required property Style style
    required property int number
    required property string name
    property bool active: false
    property bool stub: false

    spacing: style.unit * 0.55

    Rectangle {
        implicitWidth: Math.round(label.style.unit * 0.4)
        implicitHeight: implicitWidth
        color: label.active ? label.style.accent : "transparent"
        border.width: label.active ? 0 : 1
        border.color: label.style.dim
    }

    Text {
        text: "0" + label.number
        color: label.active ? label.style.text : label.style.mid
        font.family: label.style.mono
        font.pixelSize: label.style.unit * 0.62
    }

    Text {
        Layout.fillWidth: true
        text: label.name.toUpperCase()
        color: label.active ? label.style.text : label.style.dim
        font.family: label.style.mono
        font.pixelSize: label.style.unit * 0.62
        font.letterSpacing: label.style.unit * 0.1
    }

    Text {
        visible: label.stub
        text: "STUB"
        color: label.style.dim
        font.family: label.style.mono
        font.pixelSize: label.style.unit * 0.55
        font.letterSpacing: label.style.unit * 0.06
    }
}
