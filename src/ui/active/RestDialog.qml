import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Item {
    id: restDialog
    property int restSeconds: 60
    property int remainingSeconds: 0
    property bool dialogVisible: false
    property bool expanded: false
    property var endTime: null

    readonly property int collapsedHeight: Theme.layout.listItemHeight
    readonly property int expandedHeight: Theme.layout.dialogBarHeight
    readonly property int barHeight: dialogVisible ? (expanded ? expandedHeight : collapsedHeight) : 0
    readonly property bool isVisible: dialogVisible

    anchors.left: parent.left
    anchors.right: parent.right
    height: barHeight
    z: 10
    clip: true

    Behavior on height { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }

    function timeText() {
        var mins = Math.floor(remainingSeconds / 60)
        var secs = remainingSeconds % 60
        return (mins < 10 ? "0" : "") + mins + ":" + (secs < 10 ? "0" : "") + secs
    }

    function show(seconds) {
        restSeconds = seconds
        remainingSeconds = seconds
        endTime = new Date(Date.now() + seconds * 1000)
        expanded = true
        dialogVisible = true
        restTimer.start()
    }

    function hideDialog() {
        dialogVisible = false
        expanded = false
        restTimer.stop()
        remainingSeconds = 0
        endTime = null
    }

    component PlusButton: ThemedButton {
        buttonSize: Theme.button.medium
        buttonStyle: Theme.button.primary
        iconSource: Theme.icons.plus
        onClicked: {
            restDialog.restSeconds += 10
            restDialog.endTime = new Date(restDialog.endTime.getTime() + 10000)
        }
    }

    component MinusButton: ThemedButton {
        buttonSize: Theme.button.medium
        buttonStyle: Theme.button.primary
        iconSource: Theme.icons.minus
        onClicked: {
            if (restDialog.restSeconds > 10) {
                restDialog.restSeconds -= 10
                restDialog.endTime = new Date(restDialog.endTime.getTime() - 10000)
            }
        }
    }

    component ExpandButton: ThemedButton {
        iconSource: Theme.icons.expand
        buttonSize: Theme.button.smallSquare
        buttonStyle: Theme.button.primary
        onClicked: restDialog.expanded = true
    }

    component CollapseButton: ThemedButton {
        iconSource: Theme.icons.collapse
        buttonSize: Theme.button.mediumSquare
        buttonStyle: Theme.button.primary
        onClicked: restDialog.expanded = false
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
            } else {
                restDialog.remainingSeconds = diff
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.colors.dialogSurface
        radius: Theme.radius.medium
        border.width: Theme.border.thin
        border.color: Theme.colors.border

        // === Collapsed ===
        Item {
            anchors.fill: parent
            visible: !restDialog.expanded

            RowLayout {
                id: collapsedRow
                anchors.fill: parent
                anchors.margins: Theme.padding.medium
                spacing: Theme.spacing.medium

                Item { Layout.fillWidth: true }

                MinusButton { buttonSize: Theme.button.small }

                Text {
                    text: restDialog.timeText()
                    color: Theme.colors.textPrimary
                    font.pixelSize: Theme.fontSize.large
                    font.bold: true
                }

                PlusButton { buttonSize: Theme.button.small }

                Item { Layout.fillWidth: true }

                ExpandButton {}
            }
        }

        // === Expanded ===
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Theme.padding.medium
            spacing: 0
            visible: restDialog.expanded

            // Row 1: large time centered
            Text {
                Layout.fillWidth: true
                text: restDialog.timeText()
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.fontSize.huge
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            // Row 2: [ - ]  [ + ] centered
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: Theme.spacing.large

                Item { Layout.fillWidth: true }

                MinusButton {}
                PlusButton {}

                Item { Layout.fillWidth: true }

                CollapseButton {}
            }
        }
    }
}
