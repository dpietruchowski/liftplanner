import QtQuick
import LiftPlanner
import Themed.Components

Rectangle {
    id: root

    property int size: Theme.layout.indicatorSize
    property bool completed: false
    property bool highlighted: false
    property bool interactive: true

    signal toggled()

    implicitWidth: root.size
    implicitHeight: root.size
    radius: width / 2

    color: root.completed ? Theme.colors.successSurface : "transparent"
    border.width: Theme.border.medium
    border.color: root.completed ? Theme.colors.successSurface
                                 : root.highlighted ? Theme.colors.primary
                                                    : Theme.colors.borderStrong

    Behavior on color { ColorAnimation { duration: 200 } }
    Behavior on border.color { ColorAnimation { duration: 200 } }

    onCompletedChanged: {
        if (internal.settled && root.completed)
            pulse.restart()
    }

    Component.onCompleted: internal.settled = true

    QtObject {
        id: internal
        property bool settled: false
    }

    ThemedIcon {
        anchors.centerIn: parent
        width: parent.width * 0.55
        height: width
        visible: root.completed
        svgSource: Theme.icons.check
        color: Theme.colors.textPrimary
    }

    SequentialAnimation {
        id: pulse
        NumberAnimation { target: root; property: "scale"; to: 1.3; duration: 110; easing.type: Easing.OutQuad }
        NumberAnimation { target: root; property: "scale"; to: 1.0; duration: 160; easing.type: Easing.OutBack }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.interactive
        onClicked: root.toggled()
    }
}
