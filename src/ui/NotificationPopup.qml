import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Popup {
    id: notificationPopup
    width: Theme.applicationWidth * 0.85
    height: contentLayout.implicitHeight + Theme.padding.large * 2
    x: (Theme.applicationWidth - width) / 2
    y: (Theme.applicationHeight - height) / 2
    modal: true
    dim: true

    property bool iconVisible: true
    property string type: "info"
    property int buttons: NotificationPopup.Ok
    property string title: ""
    property string text: ""
    property bool copyEnabled: false
    property int textFormat: Text.PlainText

    signal accepted()
    signal rejected()

    enum Buttons {
        Ok = 1,
        Cancel = 2,
        OkCancel = 3
    }

    background: Rectangle {
        color: Theme.colors.dialogSurface
        radius: Theme.radius.medium
        opacity: Theme.dialogOpacity
    }

    Overlay.modal: Rectangle {
        color: Theme.colors.overlayLight
    }

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        anchors.margins: Theme.padding.large
        spacing: Theme.spacing.medium

        ThemedIcon {
            Layout.alignment: Qt.AlignHCenter
            visible: notificationPopup.iconVisible
            svgSource: {
                switch (notificationPopup.type) {
                    case "error": return Theme.icons.error || ""
                    case "warning": return Theme.icons.warning || ""
                    case "success": return Theme.icons.check || ""
                    default: return Theme.icons.info || ""
                }
            }
            color: {
                switch (notificationPopup.type) {
                    case "error": return Theme.colors.error
                    case "warning": return Theme.colors.warning
                    case "success": return Theme.colors.success
                    default: return Theme.colors.info
                }
            }
            width: Theme.layout.iconSizeLarge
            height: Theme.layout.iconSizeLarge
        }

        Text {
            Layout.fillWidth: true
            text: notificationPopup.title
            color: Theme.colors.textPrimary
            font.pixelSize: Theme.fontSize.large
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            visible: notificationPopup.title.length > 0
        }

        Text {
            Layout.fillWidth: true
            text: notificationPopup.text
            color: Theme.colors.textSecondary
            font.pixelSize: Theme.fontSize.medium
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            textFormat: notificationPopup.textFormat
            visible: notificationPopup.text.length > 0
        }

        ThemedButton {
            Layout.alignment: Qt.AlignHCenter
            visible: notificationPopup.copyEnabled
            text: qsTr("Copy")
            buttonSize: Theme.button.medium
            buttonStyle: Theme.button.secondary
            onClicked: ClipboardHelper.copyToClipboard(notificationPopup.text)
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Theme.spacing.medium

            ThemedButton {
                visible: (notificationPopup.buttons & NotificationPopup.Cancel) !== 0
                text: qsTr("Cancel")
                buttonSize: Theme.button.medium
                buttonStyle: Theme.button.danger
                onClicked: {
                    notificationPopup.rejected()
                    notificationPopup.close()
                }
            }

            ThemedButton {
                visible: (notificationPopup.buttons & NotificationPopup.Ok) !== 0
                text: qsTr("OK")
                buttonSize: Theme.button.medium
                buttonStyle: Theme.button.primary
                onClicked: {
                    notificationPopup.accepted()
                    notificationPopup.close()
                }
            }
        }
    }
}
