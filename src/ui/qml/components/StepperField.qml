import QtQuick
import QtQuick.Layouts
import LiftPlanner
import Themed.Components

Rectangle {
    id: root

    property string label
    property string labelName
    property string decrementName
    property string incrementName

    signal decremented()
    signal incremented()

    implicitHeight: Theme.setRow.stepperHeight
    radius: Theme.radius.medium
    color: Theme.colors.surfaceMuted
    border.width: Theme.border.thin
    border.color: Theme.colors.border

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding.xSmall
        spacing: Theme.spacing.xSmall

        ThemedButton {
            objectName: root.decrementName
            iconSource: Theme.icons.minus
            buttonSize: Theme.button.square
            buttonStyle: Theme.button.subtle
            onClicked: root.decremented()
        }

        Text {
            objectName: root.labelName
            Layout.fillWidth: true
            text: root.label
            color: Theme.colors.textMuted
            font.pixelSize: Theme.setRow.labelSize
            font.letterSpacing: Theme.setRow.labelSpacing
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        ThemedButton {
            objectName: root.incrementName
            iconSource: Theme.icons.plus
            buttonSize: Theme.button.square
            buttonStyle: Theme.button.subtle
            onClicked: root.incremented()
        }
    }
}
