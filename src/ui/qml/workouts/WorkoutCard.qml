import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root
    Layout.fillWidth: true
    Layout.preferredHeight: cardHeight

    readonly property real contentHeight: content.implicitHeight + 2 * Theme.padding.large
    property real cardHeight: contentHeight
    onContentHeightChanged: cardHeight = contentHeight

    property var workout
    property bool expanded: false
    property string dateText: ""
    property color borderColor: Theme.colors.cardBorder
    property color dateColor: Theme.colors.textMuted
    property Component expandedActions: null
    property string expandButtonName: ""
    property string titleName: ""
    property string chipPrefix: ""

    default property alias headerActions: headerActionsRow.data

    radius: Theme.radius.large
    color: Theme.colors.surface
    border.color: root.borderColor
    border.width: Theme.border.thin
    clip: true

    Behavior on cardHeight {
        NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
    }

    ColumnLayout {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.padding.large
        anchors.rightMargin: Theme.padding.large
        anchors.topMargin: Theme.padding.large
        spacing: Theme.spacing.medium

        RowLayout {
            id: headerRow
            Layout.fillWidth: true
            spacing: Theme.spacing.medium

            ThemedButton {
                objectName: root.expandButtonName
                iconSource: root.expanded ? Theme.icons.collapse : Theme.icons.expand
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.subtle
                Layout.alignment: Qt.AlignVCenter
                onClicked: root.expanded = !root.expanded
                ToolTip.visible: hovered
                ToolTip.text: root.expanded ? "Collapse" : "Expand"
                ToolTip.delay: 500
            }

            WorkoutCardTitle {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                name: root.workout ? root.workout.name : ""
                dateText: root.dateText
                dateColor: root.dateColor
                titleName: root.titleName
            }

            Row {
                id: headerActionsRow
                Layout.alignment: Qt.AlignVCenter
                spacing: Theme.spacing.medium
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.border.thin
            color: Theme.colors.divider
            visible: root.expanded
        }

        WorkoutExerciseList {
            Layout.fillWidth: true
            workout: root.workout
            expanded: root.expanded
            chipPrefix: root.chipPrefix
            opacity: root.expanded ? 1 : 0

            Behavior on opacity { NumberAnimation { duration: 200 } }
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
