import QtQuick
import QtQuick.Controls
import LiftPlanner

MouseArea {
    id: root

    property color progressColor: Theme.colors.primary
    property int progressHeight: Theme.border.medium

    signal held()

    function resetProgress() {
        holdAnimation.stop()
        progress.width = 0
    }

    onPressAndHold: root.held()
    onPressed: holdAnimation.restart()
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
