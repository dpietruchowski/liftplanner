import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

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
