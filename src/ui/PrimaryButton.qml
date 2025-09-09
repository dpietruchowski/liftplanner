import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LiftPlanner 1.0

Button {
    id: control
    property url svgIcon: ""
    property color iconTint: Theme.buttonText
    property bool circular: false
    property var buttonTheme: Theme.buttonMedium
    property var buttonStyle: Theme.buttonStylePrimary

    // implicitWidth: Math.max(buttonTheme.width, label.implicitWidth + (svgIcon ? buttonTheme.iconSize + 24 : 24))
    // implicitHeight: Math.max(buttonTheme.height, buttonTheme.iconSize + 12)
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

            ColoredSvgProvider {
                id: svgProvider
                svgOriginSource: control.svgIcon
                color: control.iconTint
            }

            Image {
                id: svg
                source: svgProvider.svgSource
                visible: control.svgIcon && control.svgIcon !== ""
                width: visible ? buttonTheme.iconSize : 0
                height: visible ? buttonTheme.iconSize : 0
                fillMode: Image.PreserveAspectFit
                smooth: true
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

    focusPolicy: Qt.StrongFocus
    Accessible.role: Accessible.Button
}
