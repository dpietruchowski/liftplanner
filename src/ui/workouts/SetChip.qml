import QtQuick
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root

    property var setData

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
    color: Theme.colors.chipBackground

    Text {
        id: text
        anchors.centerIn: parent
        text: root.label
        font.pixelSize: Theme.chip.fontSize
        color: Theme.colors.chipText
    }
}
