import QtQuick
import QtQuick.Layouts
import Themed.Components

Rectangle {
    id: root

    property string label: ""
    property int nameSize: Theme.fontSize.medium
    property color nameColor: Theme.colors.textSecondary
    property bool cardVisible: false
    property var workout: null

    readonly property string workoutName: workout ? workout.name : "---"
    readonly property string dateText: {
        if (!workout) return ""
        if (workout.endedTime && workout.endedTime.getTime() > 0) return Qt.formatDate(workout.endedTime, "d MMM")
        if (workout.startedTime && workout.startedTime.getTime() > 0) return Qt.formatDate(workout.startedTime, "d MMM")
        if (workout.plannedTime && workout.plannedTime.getTime() > 0) return Qt.formatDate(workout.plannedTime, "d MMM")
        return ""
    }

    radius: Theme.radius.large
    color: cardVisible ? Theme.colors.surface : "transparent"
    border.width: cardVisible ? Theme.border.medium : 0
    border.color: Theme.colors.primaryVariant

    implicitHeight: inner.implicitHeight

    ColumnLayout {
        id: inner
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            margins: Theme.padding.medium
        }
        spacing: 2

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: root.label
            color: Theme.colors.textSecondary
            font.pixelSize: Theme.fontSize.small
            visible: root.label.length > 0
        }

        Item {
            Layout.fillHeight: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing.small

            Text {
                text: root.dateText
                color: Theme.colors.textSecondary
                font.pixelSize: Theme.fontSize.small
                visible: root.dateText.length > 0
                Layout.alignment: Qt.AlignVCenter
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    text: root.workoutName
                    color: root.nameColor
                    font.pixelSize: root.nameSize
                    font.bold: true
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    Layout.fillWidth: true
                    text: {
                        if (!root.workout || !root.workout.exercises || root.workout.exercises.length === 0)
                            return ""
                        var exerciseNames = []
                        for (var i = 0; i < root.workout.exercises.length; i++) {
                            exerciseNames.push(root.workout.exercises[i].name)
                        }
                        return exerciseNames.join(", ")
                    }
                    color: Theme.colors.textSecondary
                    font.pixelSize: Theme.fontSize.small - 2
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                    visible: root.workout && root.workout.exercises && root.workout.exercises.length > 0
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
