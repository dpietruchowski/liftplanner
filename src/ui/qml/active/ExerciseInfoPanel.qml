import QtQuick
import QtQuick.Layouts
import Themed.Components

ThemedPanel {
    id: panel
    objectName: "exerciseInfoPanel"

    property var exercise: null

    readonly property string videoSearchUrl: exercise
        ? "https://www.youtube.com/results?search_query=" + encodeURIComponent(exercise.name)
        : ""

    panelTitle: exercise ? exercise.name : ""
    panelMessage: exercise ? exercise.description : ""

    function showExercise(item) {
        panel.exercise = item
        panel.open()
    }

    Rectangle {
        objectName: "exerciseInfoVideoRow"
        Layout.fillWidth: true
        Layout.preferredHeight: Theme.panel.rowHeight
        radius: Theme.radius.medium
        color: videoArea.pressed ? Theme.colors.surfaceAccent : Theme.colors.chipBackground
        border.width: Theme.border.thin
        border.color: Theme.colors.border

        Behavior on color { ColorAnimation { duration: 120 } }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.padding.medium
            anchors.rightMargin: Theme.padding.medium
            spacing: Theme.spacing.medium

            ThemedIcon {
                svgSource: Theme.icons.video
                color: Theme.colors.textMuted
                width: Theme.panel.rowIconSize
                height: Theme.panel.rowIconSize
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("YouTube")
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.fontSize.normal
                elide: Text.ElideRight
            }

            ThemedIcon {
                svgSource: Theme.icons.expand
                color: Theme.colors.textMuted
                width: Theme.panel.rowIconSize
                height: Theme.panel.rowIconSize
                Layout.alignment: Qt.AlignVCenter
            }
        }

        MouseArea {
            id: videoArea
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: Qt.openUrlExternally(panel.videoSearchUrl)
        }
    }

    ThemedButton {
        objectName: "exerciseInfoOkButton"
        Layout.fillWidth: true
        Layout.preferredHeight: Theme.button.wide.height
        text: qsTr("OK")
        buttonSize: Theme.button.wide
        buttonStyle: Theme.button.primary
        onClicked: panel.close()
    }
}
