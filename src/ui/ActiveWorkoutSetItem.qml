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
    color: setData.completed ? Theme.colors.success : Theme.colors.surface
    border.width: ActiveWorkoutService.currentSet === setData ? Theme.border.thick : Theme.border.thin
    border.color: ActiveWorkoutService.currentSet === setData ? Theme.colors.primaryVariant : Theme.colors.border

    Behavior on color { ColorAnimation { duration: 200 } }
    Behavior on height { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }

    clip: true

    MouseArea {
        anchors.fill: parent
        onClicked: ActiveWorkoutService.currentSet = setData
    }

    Column {
        anchors.fill: parent
        spacing: 0

        // Row 1: Set N | reps | weight | indicator
        RowLayout {
            width: parent.width
            height: Theme.layout.listItemHeight
            spacing: 0

            // "Set N" column
            Text {
                text: "Set " + (index + 1)
                font.pixelSize: Theme.fontSize.small
                font.bold: true
                color: setData.completed ? Theme.colors.buttonText : Theme.colors.textPrimary
                Layout.preferredWidth: 60
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
                    color: setData.completed ? Theme.colors.buttonText : Theme.colors.textSecondary
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
                    color: setData.completed ? Theme.colors.buttonText : Theme.colors.textSecondary
                }
            }

            Item { Layout.fillWidth: true }

            // Indicator column
            Rectangle {
                width: Theme.layout.indicatorSize
                height: Theme.layout.indicatorSize
                radius: Theme.layout.indicatorSize / 2
                color: setData.completed ? Theme.colors.buttonText : "transparent"
                border.color: setData.completed ? Theme.colors.success : Theme.colors.border
                border.width: Theme.border.medium
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: Theme.padding.medium

                MouseArea {
                    anchors.fill: parent
                    onClicked: actionsVisible = !actionsVisible
                }
            }
        }

        // Row 2 (expanded): [-][+] under reps | [-][+] under weight | [-][+] add/remove
        RowLayout {
            visible: actionsVisible
            width: parent.width
            height: Theme.button.square.size + Theme.padding.small
            spacing: 0

            // Empty space under "Set N"
            Item { Layout.preferredWidth: 60 }

            // Reps -/+ under reps column
            Item {
                Layout.preferredWidth: 80
                Layout.fillHeight: true

                RowLayout {
                    anchors.centerIn: parent
                    spacing: Theme.spacing.xsmall

                    ThemedButton {
                        iconSource: Theme.icons.minus
                        buttonSize: Theme.button.square
                        buttonStyle: Theme.button.ghost
                        onClicked: { if (setData.repetitions > 0) setData.repetitions -= 1 }
                    }

                    ThemedButton {
                        iconSource: Theme.icons.plus
                        buttonSize: Theme.button.square
                        buttonStyle: Theme.button.ghost
                        onClicked: setData.repetitions += 1
                    }
                }
            }

            // Weight -/+ under weight column
            Item {
                Layout.preferredWidth: 80
                Layout.fillHeight: true

                RowLayout {
                    anchors.centerIn: parent
                    spacing: Theme.spacing.xsmall

                    ThemedButton {
                        iconSource: Theme.icons.minus
                        buttonSize: Theme.button.square
                        buttonStyle: Theme.button.ghost
                        onClicked: { if (setData.weight >= 2.5) setData.weight -= 2.5 }
                    }

                    ThemedButton {
                        iconSource: Theme.icons.plus
                        buttonSize: Theme.button.square
                        buttonStyle: Theme.button.ghost
                        onClicked: setData.weight += 2.5
                    }
                }
            }

            Item { Layout.fillWidth: true }

            // Add/remove set under indicator
            RowLayout {
                spacing: Theme.spacing.xsmall
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: Theme.padding.medium

                ThemedButton {
                    iconSource: Theme.icons.addSet
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.ghost
                    onClicked: ActiveWorkoutService.duplicateSet(modelData)
                }

                ThemedButton {
                    iconSource: Theme.icons.removeSet
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.ghost
                    onClicked: ActiveWorkoutService.removeSet(modelData)
                }
            }
        }
    }
}
