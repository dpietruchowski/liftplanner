import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root
    Layout.fillWidth: true
    Layout.preferredHeight: Theme.layout.exerciseItemHeight
    radius: Theme.radius.medium / 2
    color: Theme.colors.background
    anchors.leftMargin: Theme.padding.medium
    anchors.rightMargin: Theme.padding.medium

    property var exercise

    RowLayout {
        id: row
        anchors.fill: parent
        spacing: Theme.spacing.medium

        Text {
            text: exercise.name
            font.pixelSize: Theme.fontSize.small
            font.bold: true
            color: Theme.colors.textPrimary
            Layout.fillWidth: true
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            text: exercise.setsToString()
            font.pixelSize: Theme.fontSize.small
            color: Theme.colors.textSecondary
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            elide: Text.ElideRight
        }
    }
}
