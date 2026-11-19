import QtQuick
import org.kde.kirigami as Kirigami

// Colours, typefaces and sizes of the panel, taken from the Plasma theme where it has them.
// See docs/design/widget.md, "Look".
QtObject {
    id: style

    readonly property color text: Kirigami.Theme.textColor
    readonly property color mid: withAlpha(text, 0.8)
    readonly property color dim: withAlpha(text, 0.6)
    readonly property color hair: withAlpha(text, 0.14)
    readonly property color accent: Kirigami.Theme.highlightColor
    readonly property color late: Kirigami.Theme.negativeTextColor
    readonly property color soon: Kirigami.Theme.neutralTextColor
    // Graphite glass: the theme's background, a little lighter, mostly opaque.
    readonly property color glass: withAlpha(Qt.lighter(Kirigami.Theme.backgroundColor, 1.3), 0.88)
    readonly property string geometric: "Poppins"
    readonly property string mono: "JetBrains Mono"
    readonly property real unit: Kirigami.Units.gridUnit
    readonly property real radius: unit

    function withAlpha(colour: color, alpha: real): color {
        return Qt.rgba(colour.r, colour.g, colour.b, alpha);
    }

    // A due date as the panel shows it: "yesterday", "today", "Saturday", "3 Dec", with its colour.
    function dueLabel(due: date): var {
        if (!due || isNaN(due.getTime())) {
            return {
                text: "",
                colour: style.dim
            };
        }
        const today = new Date();
        today.setHours(0, 0, 0, 0);
        const day = new Date(due);
        day.setHours(0, 0, 0, 0);
        const days = Math.round((day - today) / 86400000);
        if (days < -1) {
            return {
                text: `${-days} days late`,
                colour: style.late
            };
        }
        if (days === -1) {
            return {
                text: "yesterday",
                colour: style.late
            };
        }
        if (days === 0) {
            return {
                text: "today",
                colour: style.soon
            };
        }
        if (days === 1) {
            return {
                text: "tomorrow",
                colour: style.soon
            };
        }
        if (days < 7) {
            return {
                text: Qt.locale().dayName(day.getDay()),
                colour: style.mid
            };
        }
        return {
            text: day.toLocaleDateString(Qt.locale(), "d MMM"),
            colour: style.mid
        };
    }
}
