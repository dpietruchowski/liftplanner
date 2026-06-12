import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Column {
    id: exerciseDelegate
    property var exercise: modelData
    signal showExerciseInfo(var exercise)
    width: contentColumn.width
    spacing: Theme.spacing.medium / 2

    function getBorderColor() {
        if (ActiveWorkoutService.currentExercise === exercise) {
            return Theme.colors.primary
        } else {
            return Theme.colors.border
        }
    }

    function getBorderWidth() {
        if (ActiveWorkoutService.currentExercise === exercise) {
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
                iconSource: Theme.icons.info
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.ghost
                z: 1

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
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: ActiveWorkoutService.currentExercise = exercise
        }
    }

    Column {
        id: setsColumn
        width: exerciseDelegate.width
        spacing: Theme.spacing.medium / 2
        height: ActiveWorkoutService.currentExercise === exercise ? implicitHeight : 0
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
