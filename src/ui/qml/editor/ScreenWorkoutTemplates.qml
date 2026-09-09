import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components
import App.Components

Rectangle {
    id: root
    objectName: "screenWorkoutTemplates"
    color: Theme.colors.background

    property var templateToDelete

    signal closed()
    signal templateChosen(int templateId)

    function reset() {
        searchField.text = ""
        WorkoutTemplateViewModel.searchText = ""
        WorkoutTemplateViewModel.load()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding.screen
        spacing: Theme.spacing.medium

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing.medium

            ThemedButton {
                objectName: "templatesBackButton"
                iconSource: Theme.icons.back
                buttonSize: Theme.button.smallSquare
                buttonStyle: Theme.button.subtle
                onClicked: root.closed()
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("Templates")
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.fontSize.large
            }

            Text {
                objectName: "templatesResultCount"
                text: WorkoutTemplateViewModel.count
                color: Theme.colors.textMuted
                font.pixelSize: Theme.fontSize.small
            }
        }

        ThemedInput {
            id: searchField
            objectName: "templateSearchField"
            Layout.fillWidth: true
            placeholder: qsTr("Search templates")
            horizontalAlignment: Text.AlignLeft
            leftPadding: Theme.padding.large
            onTextChanged: WorkoutTemplateViewModel.searchText = text
        }

        ListView {
            id: templateList
            objectName: "workoutTemplateList"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: Theme.spacing.small
            model: WorkoutTemplateViewModel.templates

            delegate: WorkoutTemplateItem {
                objectName: "workoutTemplateItem" + index
                width: templateList.width
                workoutTemplate: modelData
                onUsed: root.templateChosen(modelData.templateId)
                onDuplicateRequested: WorkoutTemplateViewModel.duplicate(modelData.templateId)
                onRemoveRequested: {
                    root.templateToDelete = modelData.templateId
                    deleteTemplatePopup.open()
                }
            }
        }

        Text {
            objectName: "templatesEmptyHint"
            Layout.fillWidth: true
            visible: WorkoutTemplateViewModel.count === 0 && !WorkoutTemplateViewModel.loading
            horizontalAlignment: Text.AlignHCenter
            text: WorkoutTemplateViewModel.searchText.length > 0
                ? qsTr("No template matches this search.")
                : qsTr("No templates yet.\nBuild a workout and tap 'Save as template' to reuse it later.")
            color: Theme.colors.textMuted
            font.pixelSize: Theme.fontSize.small
        }
    }

    LoadingOverlay {
        objectName: "templatesLoadingOverlay"
        visible: WorkoutTemplateViewModel.loading && WorkoutTemplateViewModel.count === 0
        message: qsTr("Loading templates")
    }

    NotificationPopup {
        id: deleteTemplatePopup
        text: qsTr("Do you want to delete this template?")
        type: Notification.Type.Info
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        onAccepted: {
            if (root.templateToDelete !== undefined) {
                WorkoutTemplateViewModel.remove(root.templateToDelete)
                root.templateToDelete = undefined
            }
        }
    }
}
