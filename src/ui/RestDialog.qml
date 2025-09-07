import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LiftPlanner

Rectangle {
    id: dialog
    width: parent ? parent.width : 360
    color: Theme.surface
    radius: Theme.borderRadius
    border.color: Theme.border
    y: dialogVisible ? 0 : -implicitHeight
    Behavior on y { NumberAnimation { duration: 200 } }

    property int restSeconds: 0
    property int remainingSeconds: 0
    property bool dialogVisible: false
    property real endTime: 0
    signal dismissed()

    implicitHeight: contentLayout.implicitHeight + Theme.padding * 2

    Timer {
        id: updateTimer
        interval: 250
        repeat: true
        running: false
        onTriggered: {
            if (endTime > 0) {
                var now = Date.now()
                remainingSeconds = Math.max(0, Math.ceil((endTime - now) / 1000))
                if (remainingSeconds === 0) {
                    running = false
                    updateTimer.running = false
                }
            }
        }
    }

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        anchors.margins: Theme.padding
        spacing: Theme.spacing

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            Text {
                text: {
                    var s = remainingSeconds
                    var mm = Math.floor(s / 60)
                    var ss = s % 60
                    return (mm < 10 ? "0" + mm : mm) + ":" + (ss < 10 ? "0" + ss : ss)
                }
                font.pixelSize: Theme.fontHuge
                color: Theme.textPrimary
                horizontalAlignment: Text.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight
            spacing: Theme.spacing

            PrimaryButton {
                text: "-"
                buttonTheme: Theme.buttonMedium
                buttonStyle: Theme.buttonStylePrimary
                onClicked: {
                    endTime -= 15000
                    remainingSeconds = Math.max(0, Math.ceil((endTime - Date.now()) / 1000))
                }
            }

            PrimaryButton {
                text: "+"
                buttonTheme: Theme.buttonMedium
                buttonStyle: Theme.buttonStylePrimary
                onClicked: {
                    endTime += 15000
                    remainingSeconds = Math.ceil((endTime - Date.now()) / 1000)
                }
            }

            PrimaryButton {
                text: "OK"
                buttonTheme: Theme.buttonMedium
                buttonStyle: Theme.buttonStyleSuccess
                onClicked: dialog.hideDialog()
            }
        }
    }

    function show(seconds) {
        restSeconds = seconds || 0
        remainingSeconds = restSeconds
        endTime = Date.now() + remainingSeconds * 1000
        dialogVisible = true
        updateTimer.running = restSeconds > 0
    }

    function hideDialog() {
        dialogVisible = false
        updateTimer.running = false
        dismissed()
    }
}
