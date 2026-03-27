import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

ColumnLayout {
    id: root
    Layout.fillWidth: true
    spacing: Theme.spacing.medium
    property var workout
    property bool expanded: false

    signal deleteWorkout(var workout)

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: Theme.layout.listItemHeightLarge
        radius: Theme.radius.medium
        color: Theme.colors.surface
        border.color: Theme.colors.primary
        border.width: Theme.border.thin

        RowLayout {
            anchors.fill: parent
            anchors.margins: Theme.padding.medium
            spacing: Theme.spacing.medium

            ThemedButton {
                iconSource: expanded ? Theme.icons.collapse : Theme.icons.expand
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.primary
                onClicked: expanded = !expanded
            }

            Text {
                text: workout.name
                font.pixelSize: Theme.fontSize.medium
                font.bold: true
                color: Theme.colors.textPrimary
                Layout.fillWidth: true
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                text: Qt.formatDateTime(workout.startedTime, "dd.MM.yyyy")
                font.pixelSize: Theme.fontSize.small
                color: Theme.colors.textSecondary
                verticalAlignment: Text.AlignVCenter
            }

            ThemedButton {
                iconSource: Theme.icons.close
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.danger
                onClicked: root.deleteWorkout(workout)
            }
        }
    }

    Repeater {
        model: expanded ? workout.exercises : []
        delegate: ExerciseItem {
            Layout.fillWidth: true
            exercise: modelData
        }
    }

    Rectangle {
        Layout.fillWidth: true
        height: Theme.border.thin
        color: Theme.colors.border
    }
}
