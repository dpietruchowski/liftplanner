import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Column {
    id: exerciseDelegate
    property var exercise: modelData
    property var screen
    property int exerciseCount: 0
    signal showExerciseInfo(var exercise)
    width: contentColumn.width
    spacing: Theme.spacing.medium / 2

    readonly property bool isCurrent: ActiveWorkoutViewModel.currentExercise === exercise
    readonly property bool isExpanded: !screen.reorderMode && screen.expandedExercise === exercise
    readonly property real moveStep: Theme.layout.listItemHeight + Theme.spacing.medium / 2
    property real moveStartY: 0

    function playMove(dir) {
        moveStartY = -dir * moveStep
        moveAnim.restart()
    }

    transform: Translate { id: moveTranslate; y: 0 }

    SequentialAnimation {
        id: moveAnim
        PropertyAction { target: moveTranslate; property: "y"; value: exerciseDelegate.moveStartY }
        NumberAnimation {
            target: moveTranslate
            property: "y"
            to: 0
            duration: 220
            easing.type: Easing.OutCubic
        }
    }

    Connections {
        target: exerciseDelegate.screen
        function onExerciseMoved(ex, dir) {
            if (ex === exerciseDelegate.exercise)
                exerciseDelegate.playMove(dir)
        }
    }

    function getBorderColor() {
        if (isCurrent) {
            return Theme.colors.primary
        } else {
            return Theme.colors.border
        }
    }

    function getBorderWidth() {
        if (isCurrent) {
            return Theme.border.thick
        } else {
            return Theme.border.medium
        }
    }

    Rectangle {
        width: exerciseDelegate.width
        height: Theme.layout.listItemHeight

        radius: Theme.radius.medium
        color: Theme.colors.background

        border.color: getBorderColor()
        border.width: getBorderWidth()

        Rectangle {
            id: moveFlash
            anchors.fill: parent
            radius: parent.radius
            color: Theme.colors.primary
            opacity: 0

            SequentialAnimation {
                id: flashAnim
                running: moveAnim.running
                NumberAnimation { target: moveFlash; property: "opacity"; to: 0.3; duration: 110 }
                NumberAnimation { target: moveFlash; property: "opacity"; to: 0; duration: 220 }
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.spacing.medium
            anchors.rightMargin: Theme.spacing.medium
            spacing: Theme.spacing.medium

            Text {
                text: exercise.name

                Layout.fillWidth: true
                Layout.maximumWidth: implicitWidth + 1

                color: Theme.colors.textPrimary
                font.pixelSize: Theme.fontSize.medium
                font.bold: true
                font.letterSpacing: 1.5

                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            ThemedButton {
                objectName: "exerciseInfoButton"
                iconSource: Theme.icons.info
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.ghost
                z: 1
                visible: !screen.reorderMode

                onClicked: exerciseDelegate.showExerciseInfo(exercise)
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                width: Theme.layout.indicatorSize
                height: Theme.layout.indicatorSize
                radius: Theme.layout.indicatorSize / 2
                color: exercise.completed ? Theme.colors.success : "transparent"
                border.color: exercise.completed ? Theme.colors.success : Theme.colors.border
                border.width: Theme.border.medium
                Layout.alignment: Qt.AlignVCenter
                visible: !screen.reorderMode
            }

            RowLayout {
                spacing: Theme.spacing.xSmall
                Layout.alignment: Qt.AlignVCenter
                visible: screen.reorderMode
                z: 1

                ThemedButton {
                    iconSource: Theme.icons.moveUp
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.ghost
                    enabled: index > 0
                    onClicked: exerciseDelegate.screen.requestMove(exercise, index, -1)
                }

                ThemedButton {
                    iconSource: Theme.icons.moveDown
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.ghost
                    enabled: index < exerciseDelegate.exerciseCount - 1
                    onClicked: exerciseDelegate.screen.requestMove(exercise, index, 1)
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            z: -1
            enabled: !screen.reorderMode
            onClicked: exerciseDelegate.screen.expandedExercise = exercise
        }
    }

    Column {
        id: setsColumn
        width: exerciseDelegate.width
        spacing: Theme.spacing.medium / 2
        height: exerciseDelegate.isExpanded ? implicitHeight : 0
        visible: height > 0
        clip: true

        Behavior on height { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }

        Repeater {
            model: exercise.sets

            delegate: ActiveWorkoutSetItem {
                setData: modelData
            }
        }
    }
}
