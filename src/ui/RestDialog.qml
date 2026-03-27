import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: restDialog
    property int restSeconds: 60
    property int remainingSeconds: 0
    property bool dialogVisible: false
    property var endTime: null

    visible: dialogVisible
    width: parent.width
    height: 120
    anchors.top: parent.top
    color: Theme.colors.dialogSurface
    radius: Theme.radius.medium
    border.width: Theme.border.thin
    border.color: Theme.colors.border

    function show(seconds) {
        restSeconds = seconds
        remainingSeconds = seconds
        endTime = new Date(Date.now() + seconds * 1000)
        dialogVisible = true
        restTimer.start()
    }

    function hideDialog() {
        dialogVisible = false
        restTimer.stop()
        remainingSeconds = 0
        endTime = null
    }

    Timer {
        id: restTimer
        interval: 250
        repeat: true
        running: false
        onTriggered: {
            if (restDialog.endTime === null) return
            var diff = Math.ceil((restDialog.endTime.getTime() - Date.now()) / 1000)
            if (diff <= 0) {
                restDialog.remainingSeconds = 0
                restDialog.hideDialog()
                ActiveWorkoutService.onRestFinished()
            } else {
                restDialog.remainingSeconds = diff
            }
        }
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
                if (restDialog.restSeconds > 10) {
                    restDialog.restSeconds -= 10
                    restDialog.endTime = new Date(restDialog.endTime.getTime() - 10000)
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: {
                var mins = Math.floor(restDialog.remainingSeconds / 60)
                var secs = restDialog.remainingSeconds % 60
                return (mins < 10 ? "0" : "") + mins + ":" + (secs < 10 ? "0" : "") + secs
            }
            color: Theme.colors.textPrimary
            font.pixelSize: Theme.fontSize.huge
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        ThemedButton {
            text: "+"
            buttonSize: Theme.button.medium
            buttonStyle: Theme.button.primary
            onClicked: {
                restDialog.restSeconds += 10
                restDialog.endTime = new Date(restDialog.endTime.getTime() + 10000)
            }
        }

        ThemedButton {
            text: "OK"
            buttonSize: Theme.button.medium
            buttonStyle: Theme.button.success
            onClicked: restDialog.hideDialog()
        }
    }
}
