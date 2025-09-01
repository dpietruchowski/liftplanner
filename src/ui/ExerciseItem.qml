import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner

Rectangle {
    id: root
    Layout.fillWidth: true
    Layout.preferredHeight: 25
    radius: Theme.borderRadius / 2
    color: Theme.background
    anchors.leftMargin: Theme.padding
    anchors.rightMargin: Theme.padding

    property var exercise

    function repsSummary(sets) {
        return Array.from(sets).map(s => s.repetitions).join(", ")
    }

    RowLayout {
        id: row
        anchors.fill: parent
        spacing: Theme.spacing

        Text {
            text: exercise.name
            font.pixelSize: Theme.fontSmall
            font.bold: true
            color: Theme.textPrimary
            Layout.fillWidth: true
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            text: "Sets: " + repsSummary(exercise.sets)
            font.pixelSize: Theme.fontSmall
            color: Theme.textSecondary
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            elide: Text.ElideRight
        }
    }
}
