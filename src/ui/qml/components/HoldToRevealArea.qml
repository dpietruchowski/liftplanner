import QtQuick
import QtQuick.Controls
import Themed.Components

MouseArea {
    id: root

    property color progressColor: Theme.colors.primary
    property int progressHeight: Theme.border.medium

    property bool heldDuringThisPress: false

    signal held()
    signal tapped()

    function resetProgress() {
        holdAnimation.stop()
        progress.width = 0
    }

    onPressAndHold: {
        root.heldDuringThisPress = true
        root.held()
    }
    onPressed: {
        root.heldDuringThisPress = false
        holdAnimation.restart()
    }
    onClicked: {
        if (!root.heldDuringThisPress)
            root.tapped()
    }
    onReleased: root.resetProgress()
    onCanceled: root.resetProgress()

    Rectangle {
        id: progress
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        height: root.progressHeight
        width: 0
        radius: height / 2
        color: root.progressColor
        visible: width > 0
    }

    NumberAnimation {
        id: holdAnimation
        target: progress
        property: "width"
        from: 0
        to: root.width
        duration: Application.styleHints.mousePressAndHoldInterval
    }
}
