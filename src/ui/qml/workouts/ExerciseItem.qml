import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

ColumnLayout {
    id: root
    Layout.fillWidth: true
    spacing: Theme.spacing.xSmall

    property var exercise
    property bool flagsMeaningful: false
    property string chipPrefix: ""

    Text {
        text: exercise.name
        font.pixelSize: Theme.fontSize.normal
        font.bold: true
        color: Theme.colors.textPrimary
        Layout.fillWidth: true
        elide: Text.ElideRight
    }

    Flow {
        Layout.fillWidth: true
        spacing: Theme.chip.spacing

        Repeater {
            model: exercise.sets

            delegate: SetChip {
                objectName: root.chipPrefix.length > 0 ? root.chipPrefix + "_" + index : ""
                setData: modelData
                flagsMeaningful: root.flagsMeaningful
            }
        }
    }
}
