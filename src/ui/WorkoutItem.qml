import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner

ColumnLayout {
    Layout.fillWidth: true
    spacing: Theme.spacing
    property var workout
    property bool expanded: false

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 56
        radius: Theme.borderRadius
        color: Theme.surface
        border.color: Theme.primary
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: Theme.padding
            spacing: Theme.spacing

            PrimaryButton {
                svgIcon: expanded ? Theme.icons.collapse : Theme.icons.expand
                circular: true
                buttonTheme: Theme.buttonSquare
                buttonStyle: Theme.buttonStylePrimary
                onClicked: expanded = !expanded
            }

            Text {
                text: workout.name
                font.pixelSize: Theme.fontMedium
                font.bold: true
                color: Theme.textPrimary
                Layout.fillWidth: true
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                text: Qt.formatDateTime(workout.startedTime, "dd.MM.yyyy")
                font.pixelSize: Theme.fontSmall
                color: Theme.textSecondary
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Repeater {
        model: expanded ? workout.exercises : []
        delegate: ExerciseItem {
            Layout.fillWidth: true
            exercise: modelData
        }
    }

    Rectangle {
        Layout.fillWidth: true
        height: 1
        color: Theme.border
    }
}
