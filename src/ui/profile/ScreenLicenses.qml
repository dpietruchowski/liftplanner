import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components
import App.Components

Rectangle {
    id: root
    objectName: "screenLicenses"
    color: Theme.colors.background

    signal closed()

    component LicenseText: Text {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        textFormat: Text.PlainText
        font.pixelSize: Theme.fontSize.small
        color: Theme.colors.textMuted
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding.screen
        spacing: Theme.spacing.medium

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing.medium

            ThemedButton {
                objectName: "licensesBackButton"
                iconSource: Theme.icons.back
                buttonSize: Theme.button.smallSquare
                buttonStyle: Theme.button.subtle
                onClicked: root.closed()
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("Open source licenses")
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.fontSize.large
                elide: Text.ElideRight
            }
        }

        ScrollView {
            id: scrollView
            objectName: "licensesScrollView"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: scrollView.contentItem.width
                spacing: Theme.spacing.medium

                Text {
                    objectName: "qtAttributionText"
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSize.normal
                    color: Theme.colors.textPrimary
                    text: qsTr("%1 %2 is built with the Qt framework %3, Copyright (C) The Qt Company Ltd. "
                               + "and other contributors. Qt is used under the GNU Lesser General Public "
                               + "License version 3 and is linked dynamically, so the Qt libraries shipped "
                               + "with this app can be replaced with your own build.")
                          .arg(AppInfo.name).arg(AppInfo.version).arg(AppInfo.qtVersion)
                }

                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSize.small
                    color: Theme.colors.textMuted
                    text: qsTr("Qt source code: https://code.qt.io\nQt licensing: https://www.qt.io/licensing")
                }

                SectionHeader {
                    Layout.fillWidth: true
                    Layout.topMargin: Theme.spacing.medium
                    title: "GNU LESSER GENERAL PUBLIC LICENSE v3"
                }

                LicenseText {
                    objectName: "lgplLicenseText"
                    text: AppInfo.licenseText("LGPL-3")
                }

                SectionHeader {
                    Layout.fillWidth: true
                    Layout.topMargin: Theme.spacing.medium
                    title: "GNU GENERAL PUBLIC LICENSE v3"
                }

                LicenseText {
                    objectName: "gplLicenseText"
                    Layout.bottomMargin: Theme.spacing.medium
                    text: AppInfo.licenseText("GPL-3")
                }
            }
        }
    }
}
