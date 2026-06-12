import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.impl
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
        id: card
        property real baseHeight: Math.max(Theme.layout.listItemHeightLarge,
                                           headerRow.implicitHeight + 2 * Theme.padding.medium)
        Layout.fillWidth: true
        Layout.preferredHeight: expanded
            ? baseHeight + Theme.spacing.small + Theme.button.square.size
            : baseHeight
        radius: Theme.radius.medium
        color: Theme.colors.surface
        border.color: Theme.colors.border
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
                id: headerRow
                Layout.fillWidth: true
                spacing: Theme.spacing.medium

                ThemedButton {
                    iconSource: expanded ? Theme.icons.collapse : Theme.icons.expand
                    circular: true
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.outline
                    onClicked: expanded = !expanded
                    ToolTip.visible: hovered
                    ToolTip.text: expanded ? "Collapse" : "Expand"
                    ToolTip.delay: 500
                }

                Column {
                    Layout.fillWidth: true

                    Text {
                        text: workout ? workout.name : ""
                        font.pixelSize: Theme.fontSize.medium
                        font.bold: true
                        color: Theme.colors.textPrimary
                        elide: Text.ElideRight
                        width: parent.width
                    }

                    Text {
                        text: workout ? Qt.formatDateTime(workout.startedTime, "ddd, d MMM yyyy") : ""
                        font.pixelSize: Theme.fontSize.small
                        color: Theme.colors.textSecondary
                        visible: text.length > 0
                    }
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
                    buttonStyle: Theme.button.primary
                    onClicked: root.exportWorkout(workout)
                    ToolTip.visible: hovered
                    ToolTip.text: "Copy to clipboard"
                    ToolTip.delay: 500
                }

                ThemedButton {
                    iconSource: Theme.icons.close
                    circular: true
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.danger
                    onClicked: root.deleteWorkout(workout)
                    ToolTip.visible: hovered
                    ToolTip.text: "Delete workout"
                    ToolTip.delay: 500
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
        visible: expanded
        Layout.fillWidth: true
        height: Theme.border.thin
        color: Theme.colors.border
    }
}
