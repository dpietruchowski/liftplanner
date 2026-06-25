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
    property string dateText: ""
    property color borderColor: Theme.colors.border
    property Component expandedActions: null

    default property alias headerActions: headerActionsRow.data

    Rectangle {
        id: card
        property real baseHeight: Math.max(Theme.layout.listItemHeightLarge,
                                           headerRow.implicitHeight + 2 * Theme.padding.medium)
        Layout.fillWidth: true
        Layout.preferredHeight: (root.expanded && root.expandedActions)
            ? baseHeight + Theme.spacing.small + Theme.button.square.size
            : baseHeight
        radius: Theme.radius.medium
        color: Theme.colors.surface
        border.color: root.borderColor
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
                    iconSource: root.expanded ? Theme.icons.collapse : Theme.icons.expand
                    circular: true
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.outline
                    onClicked: root.expanded = !root.expanded
                    ToolTip.visible: hovered
                    ToolTip.text: root.expanded ? "Collapse" : "Expand"
                    ToolTip.delay: 500
                }

                WorkoutCardTitle {
                    Layout.fillWidth: true
                    name: root.workout ? root.workout.name : ""
                    dateText: root.dateText
                }

                Row {
                    id: headerActionsRow
                    spacing: Theme.spacing.medium
                }
            }

            RowLayout {
                visible: root.expanded && root.expandedActions
                Layout.fillWidth: true
                Layout.preferredHeight: Theme.button.square.size
                spacing: Theme.spacing.medium

                Item { Layout.fillWidth: true }

                Loader {
                    sourceComponent: root.expandedActions
                }
            }
        }
    }

    WorkoutExerciseList {
        Layout.fillWidth: true
        workout: root.workout
        expanded: root.expanded
    }
}
