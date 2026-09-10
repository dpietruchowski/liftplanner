import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root

    property var workoutTemplate

    signal used()
    signal duplicateRequested()
    signal removeRequested()

    implicitHeight: content.implicitHeight + Theme.padding.medium * 2
    radius: Theme.radius.medium
    color: mouseArea.pressed ? Theme.colors.surfaceAccent : Theme.colors.cardBackground
    border.width: Theme.border.thin
    border.color: Theme.colors.cardBorder

    Behavior on color { ColorAnimation { duration: 120 } }

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.margins: Theme.padding.medium
        spacing: Theme.spacing.xSmall

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing.small

            Text {
                objectName: "templateName"
                Layout.fillWidth: true
                text: root.workoutTemplate ? root.workoutTemplate.name : ""
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.fontSize.medium
                elide: Text.ElideRight
            }

            ThemedButton {
                objectName: "duplicateTemplateButton"
                iconSource: Theme.icons.copy
                buttonSize: Theme.button.smallSquare
                buttonStyle: Theme.button.subtle
                onClicked: root.duplicateRequested()
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Duplicate")
                ToolTip.delay: 500
            }

            ThemedButton {
                objectName: "removeTemplateButton"
                iconSource: Theme.icons.removeSet
                buttonSize: Theme.button.smallSquare
                buttonStyle: Theme.button.dangerSubtle
                onClicked: root.removeRequested()
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Delete")
                ToolTip.delay: 500
            }
        }

        Text {
            objectName: "templateExercises"
            Layout.fillWidth: true
            visible: text.length > 0
            text: root.workoutTemplate ? root.workoutTemplate.exerciseNames.join(" · ") : ""
            color: Theme.colors.textSecondary
            font.pixelSize: Theme.fontSize.small
            elide: Text.ElideRight
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing.small

            Text {
                objectName: "templateMeta"
                text: root.workoutTemplate
                    ? qsTr("%1 · %2").arg(Plural.counted(root.workoutTemplate.exerciseCount,
                                                         qsTr("exercise"), qsTr("exercises")))
                                     .arg(Plural.counted(root.workoutTemplate.setCount,
                                                         qsTr("set"), qsTr("sets")))
                    : ""
                color: Theme.colors.textMuted
                font.pixelSize: Theme.fontSize.xSmall
            }

            Item { Layout.fillWidth: true }

            Text {
                objectName: "templateIncomplete"
                visible: root.workoutTemplate ? !root.workoutTemplate.complete : false
                text: qsTr("missing exercises")
                color: Theme.colors.warning
                font.pixelSize: Theme.fontSize.xSmall
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        z: -1
        cursorShape: Qt.PointingHandCursor
        onClicked: root.used()
    }
}
