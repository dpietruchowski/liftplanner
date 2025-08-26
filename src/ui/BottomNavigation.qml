import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: bottomNav
    height: 60
    color: "#333"

    property var model: []
    signal itemSelected(int index)

    function printStack() {
        console.log("StackView depth:", stackView.depth)
        for (var i = 0; i < stackView.depth; i++) {
            var item = stackView.get(i)
            console.log("Index", i, ":", item)
        }
        console.log("Current item:", stackView.currentItem)
    }

    Row {
        anchors.centerIn: parent
        spacing: 25

        Repeater {
            model: bottomNav.model
            delegate: Rectangle {
                width: 70
                height: 50
                color: "transparent"

                Text {
                    anchors.centerIn: parent
                    text: modelData.label
                    color: "white"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        bottomNav.itemSelected(index)
                        if (modelData.screen && stackView && stackView.currentItem !== modelData.screen) {
                            if (stackView.depth > 0)
                                stackView.replace(modelData.screen)
                            else
                                stackView.push(modelData.screen)
                        }
                    }
                }
            }
        }
    }
}
