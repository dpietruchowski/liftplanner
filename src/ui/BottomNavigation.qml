import QtQuick
import QtQuick.Controls
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: bottomNav
    height: Theme.layout.navBarHeight
    color: Theme.colors.surface

    property var model: []
    property int currentIndex: 0
    signal itemSelected(int index)

    Row {
        anchors.centerIn: parent
        spacing: Theme.layout.navBarSpacing

        Repeater {
            model: bottomNav.model
            delegate: Rectangle {
                width: Theme.layout.navItemWidth
                height: Theme.layout.navItemHeight
                color: "transparent"

                property bool active: stackView.currentItem === modelData.screen
                property color textColor: active ? Theme.colors.primary : Theme.colors.textSecondary

                Column {
                    anchors.fill: parent

                    ThemedIcon {
                        svgSource: modelData.icon
                        color: textColor
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: modelData.label
                        color: textColor
                        font.pixelSize: Theme.fontSize.small
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
