import QtQuick 2.15
import QtQuick.Controls 2.15
import LiftPlanner 1.0

Rectangle {
    id: bottomNav
    height: 60
    color: Theme.surface

    property var model: []
    property int currentIndex: 0
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

                property bool active: stackView.currentItem === modelData.screen
                property color textColor: active ? Theme.primary : Theme.textSecondary

                Column {
                    anchors.fill: parent

                    ColoredSvgImage {
                        id: svgImage
                        svgSource: modelData.icon
                        color: textColor
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: modelData.label
                        color: textColor
                        font.bold: active === index
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        bottomNav.currentIndex = index
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
