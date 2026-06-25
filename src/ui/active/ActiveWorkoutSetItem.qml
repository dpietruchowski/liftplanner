import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: setRect
    property var setData
    property bool actionsVisible: false

    width: setsColumn.width
    height: actionsVisible ? Theme.layout.listItemHeight + Theme.button.square.size + Theme.padding.small * 2 : Theme.layout.listItemHeight
    radius: Theme.radius.medium
    color: Theme.colors.surface
    border.width: ActiveWorkoutViewModel.currentSet === setData ? Theme.border.thick : Theme.border.thin
    border.color: ActiveWorkoutViewModel.currentSet === setData ? Theme.colors.primaryVariant : Theme.colors.border

    Behavior on color { ColorAnimation { duration: 200 } }
    Behavior on height { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }

    clip: true

    MouseArea {
        id: rowMouse
        anchors.fill: parent
        onClicked: ActiveWorkoutViewModel.currentSet = setData
        onPressAndHold: setRect.actionsVisible = !setRect.actionsVisible
        onPressed: holdAnim.restart()
        onReleased: { holdAnim.stop(); holdProgress.width = 0 }
        onCanceled: { holdAnim.stop(); holdProgress.width = 0 }
    }

    NumberAnimation {
        id: holdAnim
        target: holdProgress
        property: "width"
        from: 0
        to: setRect.width
        duration: Application.styleHints.mousePressAndHoldInterval
    }

    Rectangle {
        id: holdProgress
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        height: Theme.border.thick
        width: 0
        radius: height / 2
        color: Theme.colors.primaryVariant
        visible: width > 0
    }

    Column {
        anchors.fill: parent
        spacing: 0

        // Row 1: N | reps | weight | indicator
        RowLayout {
            width: parent.width
            height: Theme.layout.listItemHeight
            spacing: 0

            // Set number column
            Text {
                text: index + 1
                font.pixelSize: Theme.fontSize.small
                font.bold: true
                color: Theme.colors.textPrimary
                horizontalAlignment: Text.AlignHCenter
                Layout.preferredWidth: 40
                Layout.alignment: Qt.AlignVCenter
                leftPadding: Theme.padding.medium
            }

            // Reps column
            Item {
                Layout.preferredWidth: 80
                Layout.fillHeight: true

                Text {
                    anchors.centerIn: parent
                    text: setData.repetitions + " reps"
                    font.pixelSize: Theme.fontSize.small
                    color: Theme.colors.textSecondary
                }
            }

            // Weight column
            Item {
                Layout.preferredWidth: 80
                Layout.fillHeight: true

                Text {
                    anchors.centerIn: parent
                    text: setData.weight + " kg"
                    font.pixelSize: Theme.fontSize.small
                    color: Theme.colors.textSecondary
                }
            }

            Item { Layout.fillWidth: true }

            // Indicator column (tap = toggle done)
            Rectangle {
                id: indicator
                width: Theme.layout.indicatorSize
                height: Theme.layout.indicatorSize
                radius: Theme.layout.indicatorSize / 2
                color: setData.completed ? Theme.colors.success : "transparent"
                border.color: setData.completed ? Theme.colors.success : Theme.colors.border
                border.width: Theme.border.medium
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: Theme.padding.medium

                Behavior on color { ColorAnimation { duration: 200 } }
                Behavior on border.color { ColorAnimation { duration: 200 } }

                SequentialAnimation {
                    id: pulseAnim
                    NumberAnimation { target: indicator; property: "scale"; to: 1.4; duration: 110; easing.type: Easing.OutQuad }
                    NumberAnimation { target: indicator; property: "scale"; to: 1.0; duration: 160; easing.type: Easing.OutBack }
                }

                Connections {
                    target: setData
                    function onCompletedChanged() {
                        if (setData.completed)
                            pulseAnim.restart()
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: ActiveWorkoutViewModel.toggleSetCompleted(setData)
                }
            }
        }

        // Row 2 (expanded): [-][+] under reps | [-][+] under weight | [-][+] add/remove
        RowLayout {
            visible: actionsVisible
            width: parent.width
            height: Theme.button.square.size + Theme.padding.small
            spacing: 0

            // Empty space under number
            Item { Layout.preferredWidth: 40 }

            // Reps -/+ under reps column
            Item {
                Layout.preferredWidth: 80
                Layout.fillHeight: true

                RowLayout {
                    anchors.centerIn: parent
                    spacing: Theme.spacing.xSmall

                    ThemedButton {
                        iconSource: Theme.icons.minus
                        buttonSize: Theme.button.square
                        buttonStyle: Theme.button.ghost
                        onClicked: {
                            if (setData.repetitions > 0) setData.repetitions -= 1
                            ActiveWorkoutViewModel.saveCurrentWorkout()
                        }
                    }

                    ThemedButton {
                        iconSource: Theme.icons.plus
                        buttonSize: Theme.button.square
                        buttonStyle: Theme.button.ghost
                        onClicked: {
                            setData.repetitions += 1
                            ActiveWorkoutViewModel.saveCurrentWorkout()
                        }
                    }
                }
            }

            // Weight -/+ under weight column
            Item {
                Layout.preferredWidth: 80
                Layout.fillHeight: true

                RowLayout {
                    anchors.centerIn: parent
                    spacing: Theme.spacing.xSmall

                    ThemedButton {
                        iconSource: Theme.icons.minus
                        buttonSize: Theme.button.square
                        buttonStyle: Theme.button.ghost
                        onClicked: {
                            if (setData.weight >= 2.5) setData.weight -= 2.5
                            ActiveWorkoutViewModel.saveCurrentWorkout()
                        }
                    }

                    ThemedButton {
                        iconSource: Theme.icons.plus
                        buttonSize: Theme.button.square
                        buttonStyle: Theme.button.ghost
                        onClicked: {
                            setData.weight += 2.5
                            ActiveWorkoutViewModel.saveCurrentWorkout()
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }

            // Add/remove set under indicator
            RowLayout {
                spacing: Theme.spacing.xSmall
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: Theme.padding.medium

                ThemedButton {
                    iconSource: Theme.icons.addSet
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.ghost
                    onClicked: ActiveWorkoutViewModel.duplicateSet(setData)
                }

                ThemedButton {
                    iconSource: Theme.icons.removeSet
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.ghost
                    onClicked: ActiveWorkoutViewModel.removeSet(setData)
                }
            }
        }
    }
}
