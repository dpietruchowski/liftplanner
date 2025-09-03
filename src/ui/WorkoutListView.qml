import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    anchors.fill: parent

    property var workouts
    property Component workoutDelegate
    property Component buttonsRow

    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.margins: Theme.padding

        ColumnLayout {
            id: columnLayout
            width: scrollView.contentItem.width
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacing

            Loader {
                sourceComponent: root.buttonsRow
            }

            Repeater {
                model: root.workouts
                delegate: root.workoutDelegate
            }
        }
    }
}
