import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: valueEditor
    property real value: 0
    property real minValue: 0
    property real maxValue: 999
    property real step: 1
    property bool dialogVisible: false

    signal confirmed(real value)

    visible: dialogVisible
    width: parent.width
    height: 120
    anchors.bottom: parent.bottom
    color: Theme.colors.dialogSurface
    radius: Theme.radius.medium
    border.width: Theme.border.thin
    border.color: Theme.colors.border

    Behavior on y {
        NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding.medium
        spacing: Theme.spacing.medium

        ThemedButton {
            text: "-"
            buttonSize: Theme.button.medium
            buttonStyle: Theme.button.primary
            onClicked: {
                if (valueEditor.value - valueEditor.step >= valueEditor.minValue)
                    valueEditor.value -= valueEditor.step
            }
        }

        Text {
            Layout.fillWidth: true
            text: valueEditor.value.toFixed(step < 1 ? 1 : 0)
            color: Theme.colors.textPrimary
            font.pixelSize: Theme.fontSize.large
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        ThemedButton {
            text: "+"
            buttonSize: Theme.button.medium
            buttonStyle: Theme.button.primary
            onClicked: {
                if (valueEditor.value + valueEditor.step <= valueEditor.maxValue)
                    valueEditor.value += valueEditor.step
            }
        }

        ThemedButton {
            text: "OK"
            buttonSize: Theme.button.medium
            buttonStyle: Theme.button.success
            onClicked: {
                valueEditor.confirmed(valueEditor.value)
                valueEditor.dialogVisible = false
            }
        }
    }
}
