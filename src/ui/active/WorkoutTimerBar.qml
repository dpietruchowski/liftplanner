import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Item {
    id: timerBar
    property bool expanded: true

    readonly property var timer: ActiveWorkoutViewModel.timer
    readonly property int adjustSeconds: 15
    readonly property int collapsedHeight: Theme.timer.compactHeight
    readonly property int expandedHeight: Theme.timer.barHeight
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

    component PhaseLabel: Text {
        text: timerBar.timer.phaseLabel.toUpperCase()
        color: Theme.colors.primary
        font.pixelSize: Theme.timer.labelSize
        font.bold: true
        font.letterSpacing: Theme.timer.labelSpacing
    }

    component AdjustButton: ThemedButton {
        property int step: 0

        buttonSize: Theme.button.medium
        buttonStyle: Theme.button.subtle
        onClicked: timerBar.timer.addSeconds(step)
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        y: -Theme.radius.large
        height: parent.height + Theme.radius.large
        color: timerBar.timer.resting ? Theme.colors.surface : Theme.colors.surfaceAccent
        radius: Theme.radius.large
        border.width: timerBar.timer.resting ? Theme.border.thin : Theme.border.medium
        border.color: timerBar.timer.resting ? Theme.colors.border : Theme.colors.primary
    }

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: timerBar.collapsedHeight
        visible: !timerBar.expanded

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.padding.screen
            anchors.rightMargin: Theme.padding.small
            spacing: Theme.spacing.small

            PhaseLabel {}

            Text {
                objectName: "timerRemainingCompact"
                text: timerBar.timer.remainingText
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.timer.compactTimeSize
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            AdjustButton {
                step: -timerBar.adjustSeconds
                iconSource: Theme.icons.minus
                buttonSize: Theme.button.square
            }

            AdjustButton {
                step: timerBar.adjustSeconds
                iconSource: Theme.icons.plus
                buttonSize: Theme.button.square
            }

            ThemedButton {
                objectName: "timerExpandButton"
                iconSource: Theme.icons.moveDown
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.subtle
                circular: true
                onClicked: timerBar.expanded = true
            }
        }
    }

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: timerBar.expandedHeight
        visible: timerBar.expanded

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Theme.padding.screen
            spacing: Theme.spacing.small

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                PhaseLabel {
                    objectName: "timerPhaseLabel"
                    anchors.left: parent.left
                    anchors.top: parent.top
                }

                Text {
                    objectName: "timerRemainingText"
                    anchors.centerIn: parent
                    text: timerBar.timer.remainingText
                    color: Theme.colors.textPrimary
                    font.pixelSize: Theme.timer.timeSize
                    font.bold: true
                }

                ThemedButton {
                    objectName: "timerCollapseButton"
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: -Theme.padding.xSmall
                    iconSource: Theme.icons.close
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.subtle
                    circular: true
                    onClicked: timerBar.expanded = false
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.spacing.small

                AdjustButton {
                    objectName: "timerMinusButton"
                    Layout.fillWidth: true
                    Layout.preferredHeight: Theme.button.medium.height
                    step: -timerBar.adjustSeconds
                    text: "−" + timerBar.adjustSeconds + " s"
                }

                ThemedButton {
                    objectName: "timerPauseButton"
                    Layout.fillWidth: true
                    Layout.preferredHeight: Theme.button.medium.height
                    buttonSize: Theme.button.medium
                    buttonStyle: Theme.button.tonal
                    text: timerBar.timer.paused ? "Resume" : "Pause"
                    onClicked: timerBar.timer.paused ? timerBar.timer.resume() : timerBar.timer.pause()
                }

                AdjustButton {
                    objectName: "timerPlusButton"
                    Layout.fillWidth: true
                    Layout.preferredHeight: Theme.button.medium.height
                    step: timerBar.adjustSeconds
                    text: "+" + timerBar.adjustSeconds + " s"
                }
            }
        }
    }
}
