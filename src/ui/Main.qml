import QtQuick
import QtQuick.Controls
import QtQuick.Window
import Themed.Components
import App.Components

ApplicationWindow {
    id: appWindow
    width: 360
    height: 640
    visible: true
    title: "Lift Planner"
    color: Theme.colors.background

    onClosing: function(close) {
        close.accepted = backHandler.shouldClose()
    }

    AppView {
        anchors.fill: parent
        systemBarColor: Theme.colors.background
        backgroundColor: Theme.colors.background

        BackHandler {
            id: backHandler
            objectName: "backHandler"
            anchors.fill: parent
            exitMessage: qsTr("Press back again to exit")
            navigateBack: function() {
                var view = mainLoader.item
                return view && typeof view.goBack === "function" && view.goBack()
            }

            Loader {
                id: mainLoader
                asynchronous: true
                objectName: "mainLoader"
                anchors.fill: parent
                source: "common/MainView.qml"
            }
        }
    }

    Component.onCompleted: {
        Theme.applicationWidth = appWindow.width
        Theme.applicationHeight = appWindow.height
    }

    onWidthChanged: Theme.applicationWidth = width
    onHeightChanged: Theme.applicationHeight = height
}
