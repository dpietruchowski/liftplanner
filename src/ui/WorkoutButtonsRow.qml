import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner

RowLayout {
    Layout.fillWidth: true
    spacing: Theme.spacing
    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

    Button {
        Layout.fillWidth: true
        text: "Generate GPT Prompt"
        onClicked: RoutineService.generateGptPrompt()
        background: Rectangle { color: Theme.primary; radius: Theme.borderRadius }
        contentItem: Text {
            anchors.centerIn: parent
            text: parent.text
            color: Theme.buttonText
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Button {
        Layout.fillWidth: true
        text: "Import from JSON"
        onClicked: RoutineService.importWorkoutsFromClipboard()
        background: Rectangle { color: Theme.primary; radius: Theme.borderRadius }
        contentItem: Text {
            anchors.centerIn: parent
            text: parent.text
            color: Theme.buttonText
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}