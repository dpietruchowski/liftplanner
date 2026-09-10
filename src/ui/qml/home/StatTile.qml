import QtQuick
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root

    property string label: ""
    property string value: ""
    property string unit: ""
    property string badge: ""
    property bool centered: false

    implicitHeight: Theme.stat.height
    radius: Theme.radius.medium
    color: Theme.colors.surface
    border.width: Theme.border.thin
    border.color: Theme.colors.border

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.stat.padding
        spacing: 0

        Text {
            Layout.fillWidth: true
            text: root.label.toUpperCase()
            elide: Text.ElideRight
            horizontalAlignment: root.centered ? Text.AlignHCenter : Text.AlignLeft
            color: Theme.colors.textMuted
            font.pixelSize: Theme.stat.labelSize
            font.letterSpacing: Theme.stat.labelSpacing
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing.xSmall

            Item {
                visible: root.centered
                Layout.fillWidth: true
            }

            Text {
                text: root.value
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.stat.valueSize
                font.bold: true
            }

            Text {
                text: root.unit
                visible: root.unit.length > 0
                color: Theme.colors.textMuted
                font.pixelSize: Theme.stat.unitSize
                Layout.alignment: Qt.AlignBaseline
            }

            Item { Layout.fillWidth: true }

            Text {
                text: root.badge
                visible: root.badge.length > 0
                color: Theme.colors.success
                font.pixelSize: Theme.stat.badgeSize
                font.bold: true
                font.letterSpacing: Theme.drum.labelSpacing
                Layout.alignment: Qt.AlignBaseline
            }
        }
    }
}
