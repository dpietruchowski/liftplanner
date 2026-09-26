import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Popup {
    id: root

    property var summary: ({})

    signal dismissed()

    width: Theme.applicationWidth * 0.85
    padding: Theme.padding.large
    x: (Theme.applicationWidth - width) / 2
    y: (Theme.applicationHeight - height) / 2
    modal: true
    dim: true
    closePolicy: Popup.NoAutoClose

    background: Rectangle {
        color: Theme.colors.dialogSurface
        radius: Theme.radius.large
        border.width: Theme.border.thin
        border.color: Theme.colors.border
        opacity: Theme.opacity.dialog
    }

    Overlay.modal: Rectangle {
        color: Theme.colors.overlayLight
    }

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        spacing: Theme.spacing.medium

        ThemedIcon {
            Layout.alignment: Qt.AlignHCenter
            svgSource: Theme.icons.check || ""
            color: Theme.colors.success
            Layout.preferredWidth: Theme.layout.iconSizeLarge
            Layout.preferredHeight: Theme.layout.iconSizeLarge
        }

        Text {
            objectName: "workoutSummaryTitle"
            Layout.fillWidth: true
            text: qsTr("Workout complete")
            color: Theme.colors.textPrimary
            font.pixelSize: Theme.fontSize.large
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Text {
            objectName: "workoutSummaryName"
            Layout.fillWidth: true
            text: root.summary.name !== undefined ? root.summary.name : ""
            color: Theme.colors.textSecondary
            font.pixelSize: Theme.fontSize.normal
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            visible: text.length > 0
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacing.small
            spacing: Theme.stat.spacing

            StatTile {
                objectName: "workoutSummaryDurationTile"
                Layout.fillWidth: true
                centered: true
                label: "time"
                value: root.summary.durationText !== undefined ? root.summary.durationText : "0s"
            }

            StatTile {
                objectName: "workoutSummarySetsTile"
                Layout.fillWidth: true
                centered: true
                label: "sets"
                value: root.summary.setsText !== undefined ? root.summary.setsText : "0/0"
            }

            StatTile {
                objectName: "workoutSummaryVolumeTile"
                Layout.fillWidth: true
                centered: true
                label: "volume"
                value: root.summary.volumeText !== undefined ? root.summary.volumeText : "0 kg"
            }
        }

        ThemedButton {
            objectName: "workoutSummaryDoneButton"
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacing.small
            text: qsTr("Done")
            buttonSize: Theme.button.medium
            buttonStyle: Theme.button.primary
            onClicked: {
                root.close()
                root.dismissed()
            }
        }
    }
}
