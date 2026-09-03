import QtQuick
import LiftPlanner 1.0
import Themed.Components

Row {
    id: root

    property var activity: []
    property int todayIndex: -1

    spacing: Theme.week.spacing

    Repeater {
        model: 7

        delegate: Column {
            id: day

            required property int index

            readonly property bool filled: root.activity[index] === true
            readonly property bool isToday: index === root.todayIndex

            spacing: Theme.week.labelGap

            Rectangle {
                objectName: "weekDot" + day.index
                width: Theme.week.cellSize
                height: Theme.week.cellSize
                radius: Theme.week.cellRadius
                color: day.filled ? Theme.colors.primary : Theme.colors.surface
                border.width: day.filled || day.isToday ? 0 : Theme.border.thin
                border.color: Theme.colors.border

                Behavior on color { ColorAnimation { duration: 250 } }

                ThemedIcon {
                    anchors.centerIn: parent
                    width: Theme.week.iconSize
                    height: Theme.week.iconSize
                    svgSource: Theme.icons.check
                    color: Theme.colors.textPrimary
                    visible: day.filled
                }

                Canvas {
                    anchors.fill: parent
                    visible: day.isToday && !day.filled

                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.reset()
                        ctx.strokeStyle = Theme.colors.borderStrong
                        ctx.lineWidth = 1.5
                        if (ctx.setLineDash)
                            ctx.setLineDash([4, 3])

                        var r = Theme.week.cellRadius
                        var o = 1
                        var w = width - 1
                        var h = height - 1

                        ctx.beginPath()
                        ctx.moveTo(o + r, o)
                        ctx.lineTo(w - r, o)
                        ctx.quadraticCurveTo(w, o, w, o + r)
                        ctx.lineTo(w, h - r)
                        ctx.quadraticCurveTo(w, h, w - r, h)
                        ctx.lineTo(o + r, h)
                        ctx.quadraticCurveTo(o, h, o, h - r)
                        ctx.lineTo(o, o + r)
                        ctx.quadraticCurveTo(o, o, o + r, o)
                        ctx.stroke()
                    }
                }
            }

            Text {
                objectName: "weekDayLabel" + day.index
                anchors.horizontalCenter: parent.horizontalCenter
                text: Qt.locale().dayName(day.index + 1, Locale.NarrowFormat)
                font.pixelSize: Theme.week.labelSize
                font.bold: day.isToday
                color: day.isToday ? Theme.colors.textPrimary : Theme.colors.textDisabled
            }
        }
    }
}
