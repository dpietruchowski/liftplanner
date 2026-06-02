import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root
    Layout.fillWidth: true
    Layout.preferredHeight: column.implicitHeight
    radius: Theme.radius.medium / 2
    color: Theme.colors.background
    anchors.leftMargin: Theme.padding.medium
    anchors.rightMargin: Theme.padding.medium

    property var exercise

    ColumnLayout {
        id: column
        anchors.fill: parent
        spacing: Theme.spacing.xsmall

        Text {
            text: exercise.name
            font.pixelSize: Theme.fontSize.small
            font.bold: true
            color: Theme.colors.textPrimary
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideRight
        }

        Text {
            text: exercise.setsToString()
            font.pixelSize: Theme.fontSize.small
            color: Theme.colors.textSecondary
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideRight
        }
    }
}
