import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Rectangle {
    id: dialog
    width: parent ? parent.width : 360
    height: 120
    color: Theme.dialogSurface
    radius: Theme.borderRadius
    border.color: Theme.border
    y: dialogVisible ? (parent ? parent.height - height : 0) : (parent ? parent.height : 0)
    Behavior on y { NumberAnimation { duration: 200 } }

    property int value: 0
    property int minValue: 0
    property int maxValue: 100
    property int step: 1
    property bool dialogVisible: false
    signal confirmed()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding
        spacing: Theme.spacing

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Theme.spacing

            PrimaryButton {
                text: "-"
                buttonTheme: Theme.buttonMedium
                buttonStyle: Theme.buttonStylePrimary
                onClicked: if (value > minValue) value -= step
            }

            Text {
                text: value.toString()
                font.pixelSize: Theme.fontLarge
                color: Theme.textPrimary
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: 60
                Layout.alignment: Qt.AlignVCenter
            }

            PrimaryButton {
                text: "+"
                buttonTheme: Theme.buttonMedium
                buttonStyle: Theme.buttonStylePrimary
                onClicked: if (value < maxValue) value += step
            }

            PrimaryButton {
                text: "OK"
                buttonTheme: Theme.buttonMedium
                buttonStyle: Theme.buttonStyleSuccess
                onClicked: { dialogVisible = false; confirmed() }
            }
        }
    }
}
