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
    signal exportWorkout(var workout)

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: expanded
            ? Theme.layout.listItemHeightLarge + Theme.spacing.small + Theme.button.square.size
            : Theme.layout.listItemHeightLarge
        radius: Theme.radius.medium
        color: Theme.colors.surface
        border.color: Theme.colors.primary
        border.width: Theme.border.thin
        clip: true

        Behavior on Layout.preferredHeight {
            NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Theme.padding.medium
            spacing: Theme.spacing.small

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: Theme.spacing.medium

                ThemedButton {
                    iconSource: expanded ? Theme.icons.collapse : Theme.icons.expand
                    circular: true
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.primary
                    onClicked: expanded = !expanded
                }

                Text {
                    text: workout ? workout.name : ""
                    font.pixelSize: Theme.fontSize.medium
                    font.bold: true
                    color: Theme.colors.textPrimary
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }

                Text {
                    text: workout ? Qt.formatDateTime(workout.startedTime, "dd.MM.yyyy") : ""
                    font.pixelSize: Theme.fontSize.small
                    color: Theme.colors.textSecondary
                    verticalAlignment: Text.AlignVCenter
                }
            }

            RowLayout {
                visible: expanded
                Layout.fillWidth: true
                Layout.preferredHeight: Theme.button.square.size
                spacing: Theme.spacing.medium

                Item { Layout.fillWidth: true }

                ThemedButton {
                    iconSource: Theme.icons.copy
                    circular: true
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.secondary
                    onClicked: root.exportWorkout(workout)
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
