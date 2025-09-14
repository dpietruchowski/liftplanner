import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LiftPlanner 1.0

Control {
    id: control
    property url svgIcon: ""
    property string text: ""
    property color iconTint: Theme.buttonText
    property bool circular: false
    property var buttonTheme: Theme.buttonMedium
    property var buttonStyle: Theme.buttonStylePrimary
    signal clicked()
    property bool down: mouseArea.pressed

    implicitWidth: buttonTheme.width
    implicitHeight: buttonTheme.height

    background: Rectangle {
        id: bg
        radius: circular ? Math.min(control.implicitHeight, control.implicitWidth) / 2 : Theme.borderRadius
        border.width: 1
        border.color: control.down ? buttonStyle.border : buttonStyle.border
        color: control.enabled ?
               (control.down ? buttonStyle.pressed :
               (control.hovered ? buttonStyle.hovered : buttonStyle.background)) :
               Theme.surface
        smooth: true
        Behavior on color { ColorAnimation { duration: 120 } }
        Behavior on border.color { ColorAnimation { duration: 120 } }
    }

    contentItem: Item {
        anchors.centerIn: parent
        width: parent.width
        height: parent.height

        Row {
            spacing: 8
            anchors.centerIn: parent

            ColoredSvgImage {
                id: svgImage
                visible: control.svgIcon && control.svgIcon !== ""
                width: visible ? buttonTheme.iconSize : 0
                height: visible ? buttonTheme.iconSize : 0
                svgSource: control.svgIcon
                color: control.iconTint
            }

            Text {
                id: label
                text: control.text
                font.pixelSize: buttonTheme.fontSize
                color: buttonStyle.text
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: control.clicked()
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
    }

    focusPolicy: Qt.StrongFocus
    Accessible.role: Accessible.Button
}
