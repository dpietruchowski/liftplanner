import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components
import App.Components

Rectangle {
    id: root
    objectName: "screenExercisePicker"
    color: Theme.colors.background

    signal exerciseSelected(var definition)
    signal cancelled()

    function reset() {
        searchField.text = ""
        filterPanel.reset()
        ExerciseCatalogViewModel.clearFilters()
        ExerciseCatalogViewModel.load()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding.screen
        spacing: Theme.spacing.medium

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing.medium

            ThemedButton {
                objectName: "pickerBackButton"
                iconSource: Theme.icons.back
                buttonSize: Theme.button.smallSquare
                buttonStyle: Theme.button.subtle
                onClicked: root.cancelled()
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("Add exercise")
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.fontSize.large
            }

            Text {
                objectName: "pickerResultCount"
                text: ExerciseCatalogViewModel.count
                color: Theme.colors.textMuted
                font.pixelSize: Theme.fontSize.small
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing.small

            ThemedInput {
                id: searchField
                objectName: "exerciseSearchField"
                Layout.fillWidth: true
                placeholder: qsTr("Search exercises")
                horizontalAlignment: Text.AlignLeft
                leftPadding: Theme.padding.large
                onTextChanged: ExerciseCatalogViewModel.searchText = text
            }

            ThemedButton {
                objectName: "openFiltersButton"
                iconSource: Theme.icons.menu
                buttonSize: Theme.button.icon
                buttonStyle: ExerciseCatalogViewModel.filtered ? Theme.button.tonal
                                                               : Theme.button.subtle
                onClicked: filterPanel.open()
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Filters")
                ToolTip.delay: 500
            }
        }

        ListView {
            id: exerciseList
            objectName: "exercisePickerList"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: Theme.spacing.small
            model: ExerciseCatalogViewModel.exercises

            delegate: ExercisePickerItem {
                objectName: "exercisePickerItem" + index
                width: exerciseList.width
                definition: modelData
                onClicked: root.exerciseSelected(modelData)
            }
        }

        Text {
            objectName: "exercisePickerEmpty"
            Layout.fillWidth: true
            visible: ExerciseCatalogViewModel.count === 0 && !ExerciseCatalogViewModel.loading
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("No exercise matches these filters.")
            color: Theme.colors.textMuted
            font.pixelSize: Theme.fontSize.small
        }
    }

    LoadingOverlay {
        objectName: "exercisePickerLoadingOverlay"
        visible: ExerciseCatalogViewModel.loading && ExerciseCatalogViewModel.count === 0
        message: qsTr("Loading exercises")
    }

    ExerciseFilterPanel {
        id: filterPanel
        onCleared: root.reset()
    }
}
