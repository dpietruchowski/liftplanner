import QtQuick
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root

    property var setData
    property bool flagsMeaningful: false
    property bool editable: false

    readonly property bool performed: setData ? setData.completed
                                                || setData.performed(root.flagsMeaningful)
                                              : true

    readonly property string label: {
        if (!setData)
            return ""
        var load = setData.secondaryAdjustable && parseFloat(setData.secondaryText) !== 0
                ? setData.secondaryText : ""
        if (load.length === 0)
            return setData.primaryText
        if (setData.metric === "reps")
            return setData.repetitions + " × " + load
        return setData.primaryText + " · " + load
    }

    width: text.implicitWidth + 2 * Theme.chip.padding
    height: Theme.chip.height
    radius: Theme.chip.radius
    color: root.performed ? Theme.colors.chipBackground : "transparent"
    border.width: root.performed ? 0 : Theme.border.thin
    border.color: Theme.colors.border

    Text {
        id: text
        anchors.centerIn: parent
        text: root.label
        font.pixelSize: Theme.chip.fontSize
        font.strikeout: !root.performed
        color: root.performed ? Theme.colors.chipText : Theme.colors.textFaint
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.editable && root.setData
        onClicked: WorkoutHistoryViewModel.requestSetToggle(root.setData)
    }
}
