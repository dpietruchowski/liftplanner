import QtQuick
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Item {
    id: root

    property var entries: []
    property int defaultIndex: 0
    property int currentIndex: 0

    readonly property int fullHeight: 2 * Theme.drum.rowHeight + Theme.drum.rowHeightLarge
                                      + 2 * Theme.drum.rowSpacing
    readonly property real fitScale: height > 0 ? Math.min(1, height / fullHeight) : 1

    readonly property int slotHeight: Math.round(Theme.drum.rowHeight * fitScale)
    readonly property int slotHeightLarge: Math.round(Theme.drum.rowHeightLarge * fitScale)
    readonly property int slotSpacing: Math.round(Theme.drum.rowSpacing * fitScale)
    readonly property int slotStep: slotHeight + slotSpacing

    readonly property real offsetTarget: slotStep - currentIndex * slotStep
    property real offset: offsetTarget

    implicitHeight: fullHeight
    clip: true

    function focusEntry(index) {
        if (index < 0 || index >= entries.length)
            return
        currentIndex = index
    }

    function resetIndex() {
        currentIndex = Math.max(0, Math.min(defaultIndex, entries.length - 1))
    }

    onEntriesChanged: resetIndex()
    onDefaultIndexChanged: resetIndex()
    onOffsetTargetChanged: offset = offsetTarget

    Behavior on offset {
        NumberAnimation { duration: 260; easing.type: Easing.OutCubic }
    }

    Column {
        y: root.offset
        width: root.width
        spacing: root.slotSpacing

        Repeater {
            model: root.entries

            delegate: WorkoutDrumItem {
                id: item

                required property var modelData
                required property int index

                readonly property bool isCurrent: index === root.currentIndex

                objectName: "drumItem_" + index
                nameObjectName: "drumItemName_" + index
                x: boxed ? Theme.drum.cardInset : 0
                width: root.width - 2 * x
                height: isCurrent ? root.slotHeightLarge : root.slotHeight

                workout: modelData.workout
                label: modelData.label
                highlighted: isCurrent
                boxed: isCurrent && modelData.kind === "planned"
                dotted: isCurrent && modelData.kind === "current"
                accent: modelData.kind === "current" ? Theme.colors.success : Theme.colors.primary
                accentText: modelData.kind === "current" ? Theme.colors.success
                                                         : Theme.colors.primaryLight

                Behavior on height {
                    NumberAnimation { duration: 260; easing.type: Easing.OutCubic }
                }

                Behavior on x {
                    NumberAnimation { duration: 260; easing.type: Easing.OutCubic }
                }

                MouseArea {
                    anchors.fill: parent
                    enabled: !item.isCurrent
                    onClicked: root.focusEntry(item.index)
                }
            }
        }
    }
}
