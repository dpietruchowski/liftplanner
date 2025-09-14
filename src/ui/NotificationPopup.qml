import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LiftPlanner 1.0

Popup {
    id: root
    modal: true
    dim: true
    closePolicy: Popup.CloseOnEscape
    anchors.centerIn: parent
    width: Math.min(parent.width * 0.9, 360)
    height: 0.5 * parent.height

    property int type
    property int buttons
    property string title: ""
    property string text: ""
    property bool copyEnabled: false

    signal accepted()
    signal rejected()

    background: Rectangle {
        color: Theme.dialogSurface
        radius: Theme.borderRadius
        opacity: Theme.dialogOpacity
    }

    contentItem: Item {
        anchors.fill: parent

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.margins: Theme.padding
            spacing: Theme.spacing

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight

                PrimaryButton {
                    visible: root.copyEnabled
                    svgIcon: Theme.icons.copy
                    circular: true
                    buttonTheme: Theme.buttonSquare
                    buttonStyle: Theme.buttonStylePrimary
                    onClicked: ClipboardHelper.setText(root.text)
                }
            }

            Item {
                Layout.fillHeight: true
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignVCenter
                spacing: Theme.spacing

                ColoredSvgImage {
                    id: icon
                    width: 48
                    height: 48
                    svgSource: {
                        switch (root.type) {
                            case Notification.Type.Error: return Theme.icons.error
                            case Notification.Type.Warning: return Theme.icons.warning
                            case Notification.Type.Success: return Theme.icons.success
                            case Notification.Type.Info: return Theme.icons.info
                            default: return Theme.icons.info
                        }
                    }
                    color: {
                        switch (root.type) {
                            case Notification.Type.Error: return Theme.error
                            case Notification.Type.Warning: return Theme.warning
                            case Notification.Type.Success: return Theme.success
                            case Notification.Type.Info: return Theme.info
                            default: return Theme.primaryVariant
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Theme.spacing / 2

                    Text {
                        visible: root.title !== ""
                        text: root.title
                        color: Theme.textPrimary
                        font.pixelSize: Theme.fontLarge
                        font.bold: true
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    Text {
                        text: root.text
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontMedium
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                spacing: Theme.spacing

                PrimaryButton {
                    visible: root.buttons & Notification.Button.Cancel
                    text: "Cancel"
                    Layout.preferredWidth: Theme.buttonMedium.width
                    buttonTheme: Theme.buttonMedium
                    buttonStyle: Theme.buttonStyleDanger
                    onClicked: {
                        root.rejected()
                        root.close()
                    }
                }

                PrimaryButton {
                    visible: root.buttons & Notification.Button.Ok
                    text: "OK"
                    Layout.preferredWidth: Theme.buttonMedium.width
                    buttonTheme: Theme.buttonMedium
                    buttonStyle: Theme.buttonStylePrimary
                    onClicked: {
                        root.accepted()
                        root.close()
                    }
                }
            }
        }
    }
}
