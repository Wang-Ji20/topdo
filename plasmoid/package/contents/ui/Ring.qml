import QtQuick
import QtQuick.Shapes

// A progress ring: a faint full circle, and an arc from the top, clockwise, for `fraction`.
Item {
    id: ring

    property real fraction: 0
    property color track: "grey"
    property color colour: "white"
    property real thickness: 3

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: ring.track
            strokeWidth: ring.thickness
            fillColor: "transparent"

            PathAngleArc {
                centerX: ring.width / 2
                centerY: ring.height / 2
                radiusX: (ring.width - ring.thickness) / 2
                radiusY: radiusX
                startAngle: 0
                sweepAngle: 360
            }
        }

        ShapePath {
            strokeColor: ring.colour
            strokeWidth: ring.thickness
            fillColor: "transparent"
            capStyle: ShapePath.FlatCap

            PathAngleArc {
                centerX: ring.width / 2
                centerY: ring.height / 2
                radiusX: (ring.width - ring.thickness) / 2
                radiusY: radiusX
                startAngle: -90
                sweepAngle: 360 * ring.fraction
            }
        }
    }
}
