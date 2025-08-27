import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LiftPlanner

Item {
    id: root
    anchors.fill: parent
    property bool dialogVisible: false
    property int restSeconds: 0
    property int remainingSeconds: 0
    property string message: ""
    property string exerciseName: ""
    property bool error: false
    property bool running: false
    property bool finished: false
    signal dismissed()

    function show(seconds, msg, isError) {
        restSeconds = seconds || 0
        remainingSeconds = restSeconds
        message = msg || ""
        exerciseName = message
        error = !!isError
        finished = false
        dialogVisible = true
        if (restSeconds > 0) start()
    }

    function hideDialog() {
        reset()
        dialogVisible = false
        dismissed()
    }

    function start() {
        if (remainingSeconds <= 0) remainingSeconds = restSeconds
        running = true
        timer.running = true
    }

    function pause() {
        running = false
        timer.running = false
    }

    function stopAndClose() {
        running = false
        timer.running = false
        hideDialog()
    }

    function reset() {
        remainingSeconds = restSeconds
        running = false
        timer.running = false
        finished = false
    }

    Timer {
        id: timer
        interval: 1000
        repeat: true
        running: false
        onTriggered: {
            if (remainingSeconds > 0) {
                remainingSeconds--
                // pulseAnim.start()
                if (remainingSeconds === 0) {
                    running = false
                    timer.running = false
                    finished = true
                }
            } else {
                running = false
                timer.running = false
                finished = true
            }
        }
    }

    Rectangle {
        id: overlay
        anchors.fill: parent
        color: "#000000A0"
        visible: root.dialogVisible

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
            hoverEnabled: true
            onClicked: {
            }
        }

        Rectangle {
            id: container
            anchors.fill: parent
            color: "transparent"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 20

                Rectangle {
                    id: card
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: Theme.borderRadius
                    color: Theme.surface
                    border.color: Theme.border
                    border.width: 1
                    opacity: 0.98

                    Column {
                        anchors.fill: parent
                        anchors.margins: 24
                        spacing: 16

                        Text {
                            id: header
                            text: root.exerciseName || root.message || "Rest"
                            font.pixelSize: Theme.fontLarge
                            font.bold: true
                            color: root.error ? Theme.error : Theme.textPrimary
                            horizontalAlignment: Text.AlignHCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Item { height: 8 }

                        Item {
                            id: timerArea
                            width: parent.width
                            height: parent.height - header.height - 140

                            Rectangle {
                                id: timerCircle
                                width: Math.min(parent.width * 0.8, 360)
                                height: timerCircle.width
                                radius: timerCircle.width / 2
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: parent.verticalCenter
                                color: Theme.background
                                border.color: Theme.primary
                                border.width: 2
                                scale: timerScale

                                Rectangle {
                                    anchors.fill: parent
                                    color: "transparent"
                                    radius: parent.radius
                                    layer.enabled: true
                                }

                                Column {
                                    anchors.centerIn: parent
                                    spacing: 8

                                    Text {
                                        id: timerLabel
                                        text: {
                                            if (finished) {
                                                return "Do your workout!"
                                            }

                                            var s = root.remainingSeconds || root.restSeconds || 0
                                            var mm = Math.floor(s / 60)
                                            var ss = s % 60
                                            return (mm < 10 ? "0" + mm : mm) + ":" + (ss < 10 ? "0" + ss : ss)
                                        }
                                        font.pixelSize: finished ? 32 : 84
                                        font.bold: true
                                        color: Theme.primary
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                }
                            }

                            Row {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottom: parent.bottom
                                spacing: 20

                                Button {
                                    text: "-15s"
                                    onClicked: {
                                        root.remainingSeconds = Math.max(0, root.remainingSeconds - 15)
                                        if (root.remainingSeconds === 0) {
                                            root.finished = true
                                            root.running = false
                                            timer.running = false
                                        }
                                    }
                                }

                                Button {
                                    text: "+15s"
                                    onClicked: {
                                        root.remainingSeconds = root.remainingSeconds + 15
                                    }
                                }
                            }
                        }

                        RowLayout {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 16

                            Button {
                                text: "Stop"
                                onClicked: {
                                    root.stopAndClose()
                                }
                            }

                            Button {
                                visible: root.finished
                                text: "OK"
                                onClicked: {
                                    root.hideDialog()
                                }
                            }

                            Button {
                                visible: !root.finished
                                text: root.running ? "Pause" : "Start"
                                onClicked: {
                                    if (!root.running) root.start(); else root.pause()
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    property real timerScale: 1

    // SequentialAnimation {
    //     id: pulseAnim
    //     NumberAnimation { target: root; property: "timerScale"; to: 1.12; duration: 120 }
    //     NumberAnimation { target: root; property: "timerScale"; to: 1.0; duration: 160 }
    // }

    onRestSecondsChanged: {
        remainingSeconds = restSeconds
    }

    Component.onCompleted: reset()
}
