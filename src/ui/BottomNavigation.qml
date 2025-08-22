import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: bottomNav
    height: 60
    color: "#333"

    signal itemSelected(int index)

    Row {
        anchors.centerIn: parent
        spacing: 40

        Repeater {
            model: ["Home", "Workout", "History"]
            delegate: Rectangle {
                width: 80
                height: 50
                color: "transparent"
                Text {
                    anchors.centerIn: parent
                    text: modelData
                    color: "white"
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: bottomNav.itemSelected(index)
                }
            }
        }
    }
}