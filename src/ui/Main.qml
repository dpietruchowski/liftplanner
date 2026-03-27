import QtQuick
import QtQuick.Controls
import QtQuick.Window
import Themed.Components

ApplicationWindow {
    id: appWindow
    width: 360
    height: 640
    visible: true
    title: "Lift Planner"

    Loader {
        id: mainLoader
        asynchronous: true
        objectName: "mainLoader"
        anchors.fill: parent
        source: "MainView.qml"
    }

    Component.onCompleted: {
        Theme.applicationWidth = appWindow.width
        Theme.applicationHeight = appWindow.height
    }

    onWidthChanged: Theme.applicationWidth = width
    onHeightChanged: Theme.applicationHeight = height
}
