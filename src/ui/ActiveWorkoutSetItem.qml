import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner

Rectangle {
    property var setData
    Layout.fillWidth: true
    height: 60
    radius: Theme.borderRadius
    border.width: ActiveWorkoutService.currentSet === setData ? 3 : 1

    color: setData.completed ? Theme.success : Theme.surface
    border.color: ActiveWorkoutService.currentSet === setData ? Theme.primaryVariant : Theme.border

    Behavior on color { ColorAnimation { duration: 200 } }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding
        spacing: Theme.spacing

        Text {
            text: "Set " + (index + 1)
            font.pixelSize: Theme.fontSmall
            font.bold: true
            color: setData.completed ? Theme.buttonText : Theme.textPrimary
            Layout.preferredWidth: 60
        }

        Text {
            text: setData.repetitions + " reps"
            font.pixelSize: Theme.fontSmall
            color: setData.completed ? Theme.buttonText : Theme.textSecondary
            Layout.preferredWidth: 80
        }

        Text {
            text: setData.weight + " kg"
            font.pixelSize: Theme.fontSmall
            color: setData.completed ? Theme.buttonText : Theme.textSecondary
            Layout.preferredWidth: 80
        }

        Item { Layout.fillWidth: true }

        Rectangle {
            width: 20
            height: 20
            radius: 10
            color: setData.completed ? Theme.buttonText : "transparent"
            border.color: setData.completed ? Theme.success : Theme.border
            border.width: 2
            Layout.alignment: Qt.AlignVCenter
        }
    }
}
