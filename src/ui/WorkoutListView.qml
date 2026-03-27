import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Themed.Components

Item {
    id: root
    anchors.fill: parent

    property var workouts
    property Component workoutDelegate
    property Component buttonsRow

    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.margins: Theme.padding.medium

        ColumnLayout {
            id: columnLayout
            width: scrollView.contentItem.width
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacing.medium

            Loader {
                sourceComponent: root.buttonsRow
                Layout.fillWidth: true
            }

            Repeater {
                model: root.workouts
                delegate: root.workoutDelegate
            }
        }
    }
}
