import QtQuick
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

ThemedPanel {
    id: panel
    objectName: "exerciseFilterPanel"

    readonly property var regionOptions: ["", "chest", "back", "shoulders", "arms", "legs", "core", "neck"]
    readonly property var equipmentOptions: ["", "barbell", "dumbbell", "kettlebell", "machine",
        "cable", "smith_machine", "trap_bar", "bodyweight", "band", "sled", "treadmill", "bike",
        "rower", "other"]
    readonly property var kindOptions: ["", "strength", "bodyweight", "cardio", "interval", "mobility"]

    panelTitle: qsTr("Filters")

    signal cleared()

    function labelled(options) {
        return options.map(function(value) { return value === "" ? qsTr("Any") : value })
    }

    function reset() {
        regionFilter.currentIndex = 0
        equipmentFilter.currentIndex = 0
        kindFilter.currentIndex = 0
    }

    component FilterRow: RowLayout {
        id: filterRow

        property string label: ""

        Layout.fillWidth: true
        spacing: Theme.spacing.medium

        Text {
            Layout.preferredWidth: Theme.panel.filterLabelWidth
            text: filterRow.label
            color: Theme.colors.textSecondary
            font.pixelSize: Theme.fontSize.normal
        }
    }

    FilterRow {
        label: qsTr("Region")

        ThemedComboBox {
            id: regionFilter
            objectName: "regionFilter"
            Layout.fillWidth: true
            model: panel.labelled(panel.regionOptions)
            onActivated: ExerciseCatalogViewModel.region = panel.regionOptions[currentIndex]
        }
    }

    FilterRow {
        label: qsTr("Equipment")

        ThemedComboBox {
            id: equipmentFilter
            objectName: "equipmentFilter"
            Layout.fillWidth: true
            model: panel.labelled(panel.equipmentOptions)
            onActivated: ExerciseCatalogViewModel.equipment = panel.equipmentOptions[currentIndex]
        }
    }

    FilterRow {
        label: qsTr("Type")

        ThemedComboBox {
            id: kindFilter
            objectName: "kindFilter"
            Layout.fillWidth: true
            model: panel.labelled(panel.kindOptions)
            onActivated: ExerciseCatalogViewModel.kind = panel.kindOptions[currentIndex]
        }
    }

    Text {
        objectName: "filterPanelMatchCount"
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("%1 matching exercises").arg(ExerciseCatalogViewModel.count)
        color: Theme.colors.textMuted
        font.pixelSize: Theme.fontSize.small
    }

    ThemedButton {
        objectName: "clearFiltersButton"
        Layout.fillWidth: true
        Layout.preferredHeight: Theme.button.wide.height
        enabled: ExerciseCatalogViewModel.filtered
        text: qsTr("Clear filters")
        buttonSize: Theme.button.wide
        buttonStyle: Theme.button.subtle
        onClicked: panel.cleared()
    }

    ThemedButton {
        objectName: "applyFiltersButton"
        Layout.fillWidth: true
        Layout.preferredHeight: Theme.button.wide.height
        text: qsTr("Done")
        buttonSize: Theme.button.wide
        buttonStyle: Theme.button.primary
        onClicked: panel.close()
    }
}
