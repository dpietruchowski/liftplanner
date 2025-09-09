// ActiveWorkoutSetItem.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0

Rectangle {
    id: setRect
    property var setData
    property bool actionsVisible: false
    signal editRepetitions(var setData)
    signal editWeight(var setData)

    width: setsColumn.width
    height: actionsVisible ? 100 : 60
    radius: Theme.borderRadius
    color: setData.completed ? Theme.success : Theme.surface
    border.width: ActiveWorkoutService.currentSet === setData ? 3 : 1
    border.color: ActiveWorkoutService.currentSet === setData ? Theme.primaryVariant : Theme.border

    Behavior on color { ColorAnimation { duration: 200 } }

    MouseArea {
        anchors.fill: parent
        onClicked: ActiveWorkoutService.currentSet = setData
    }

    Column {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        RowLayout {
            id: mainRow
            width: parent.width
            height: 60
            spacing: Theme.spacing
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Theme.padding

            Text {
                text: "Set " + (index + 1)
                font.pixelSize: Theme.fontSmall
                font.bold: true
                color: setData.completed ? Theme.buttonText : Theme.textPrimary
                Layout.preferredWidth: 60
                Layout.alignment: Qt.AlignVCenter
            }

            Item {
                Layout.preferredWidth: 80
                Layout.fillHeight: true

                Text {
                    text: setData.repetitions + " reps"
                    font.pixelSize: Theme.fontSmall
                    color: setData.completed ? Theme.buttonText : Theme.textSecondary
                    anchors.centerIn: parent
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        ActiveWorkoutService.currentSet = setData
                        editRepetitions(setData)
                    }
                }
            }

            Item {
                Layout.preferredWidth: 80
                Layout.fillHeight: true

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        ActiveWorkoutService.currentSet = setData
                        editWeight(setData)
                    }
                }

                Text {
                    text: setData.weight + " kg"
                    font.pixelSize: Theme.fontSmall
                    color: setData.completed ? Theme.buttonText : Theme.textSecondary
                    anchors.centerIn: parent
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Rectangle {
                width: 20
                height: 20
                radius: 10
                color: setData.completed ? Theme.buttonText : "transparent"
                border.color: setData.completed ? Theme.success : Theme.border
                border.width: 2
                Layout.alignment: Qt.AlignVCenter

                MouseArea {
                    anchors.fill: parent
                    onClicked: actionsVisible = !actionsVisible
                }
            }
        }

        RowLayout {
            id: actionsRow
            spacing: Theme.spacing
            visible: actionsVisible
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Theme.padding

            Item { Layout.fillWidth: true }

            PrimaryButton {
                 svgIcon: Theme.icons.addSet
                 circular: true
                 buttonTheme: Theme.buttonSquare
                 buttonStyle: Theme.buttonStylePrimary
                 onClicked: {}
            }

            PrimaryButton {
                 svgIcon: Theme.icons.removeSet
                 circular: true
                 buttonTheme: Theme.buttonSquare
                 buttonStyle: Theme.buttonStylePrimary
                 onClicked: {}
            }
        }
    }
}
