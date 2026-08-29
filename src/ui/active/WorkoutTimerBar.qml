import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Item {
    id: timerBar
    property bool expanded: true

    readonly property var timer: ActiveWorkoutViewModel.timer
    readonly property int collapsedHeight: Theme.layout.listItemHeight
    readonly property int expandedHeight: Theme.layout.dialogBarHeight
    readonly property bool isVisible: timer.running
    readonly property int barHeight: isVisible ? (expanded ? expandedHeight : collapsedHeight) : 0

    anchors.left: parent.left
    anchors.right: parent.right
    height: barHeight
    z: 10
    clip: true

    Behavior on height { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }

    Connections {
        target: timerBar.timer
        function onPhaseChanged() {
            if (timerBar.timer.running)
                timerBar.expanded = true
        }
    }

    component PlusButton: ThemedButton {
        buttonSize: Theme.button.medium
        buttonStyle: Theme.button.primary
        iconSource: Theme.icons.plus
        onClicked: timerBar.timer.addSeconds(10)
    }

    component MinusButton: ThemedButton {
        buttonSize: Theme.button.medium
        buttonStyle: Theme.button.primary
        iconSource: Theme.icons.minus
        onClicked: timerBar.timer.addSeconds(-10)
    }

    component PauseButton: ThemedButton {
        objectName: "timerPauseButton"
        buttonSize: Theme.button.medium
        buttonStyle: Theme.button.outline
        text: timerBar.timer.paused ? "Resume" : "Pause"
        onClicked: timerBar.timer.paused ? timerBar.timer.resume() : timerBar.timer.pause()
    }

    component ExpandButton: ThemedButton {
        iconSource: Theme.icons.expand
        buttonSize: Theme.button.smallSquare
        buttonStyle: Theme.button.primary
        onClicked: timerBar.expanded = true
    }

    component CollapseButton: ThemedButton {
        iconSource: Theme.icons.collapse
        buttonSize: Theme.button.mediumSquare
        buttonStyle: Theme.button.primary
        onClicked: timerBar.expanded = false
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.colors.dialogSurface
        radius: Theme.radius.medium
        border.width: Theme.border.thin
        border.color: timerBar.timer.resting ? Theme.colors.border : Theme.colors.primaryVariant

        Item {
            anchors.fill: parent
            visible: !timerBar.expanded

            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.padding.medium
                spacing: Theme.spacing.medium

                Item { Layout.fillWidth: true }

                MinusButton { buttonSize: Theme.button.small }

                Text {
                    objectName: "timerRemainingCompact"
                    text: timerBar.timer.remainingText
                    color: Theme.colors.textPrimary
                    font.pixelSize: Theme.fontSize.large
                    font.bold: true
                }

                PlusButton { buttonSize: Theme.button.small }

                Item { Layout.fillWidth: true }

                ExpandButton {}
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Theme.padding.medium
            spacing: 0
            visible: timerBar.expanded

            Text {
                objectName: "timerPhaseLabel"
                Layout.fillWidth: true
                text: timerBar.timer.phaseLabel
                color: Theme.colors.textSecondary
                font.pixelSize: Theme.fontSize.small
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                objectName: "timerRemainingText"
                Layout.fillWidth: true
                text: timerBar.timer.remainingText
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.fontSize.huge
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: Theme.spacing.large

                Item { Layout.fillWidth: true }

                MinusButton {}
                PauseButton {}
                PlusButton {}

                Item { Layout.fillWidth: true }

                CollapseButton {}
            }
        }
    }
}
