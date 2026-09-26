import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Column {
    id: exerciseDelegate
    property var exercise: modelData
    property int exerciseIndex: 0
    property var screen
    property int exerciseCount: 0
    signal showExerciseInfo(var exercise)
    width: contentColumn.width
    spacing: Theme.spacing.small

    readonly property bool isCurrent: ActiveWorkoutViewModel.currentExercise === exercise
    readonly property bool hasPreviousSession: exercise && exercise.previousDate
                                               && !isNaN(exercise.previousDate.getTime())
    readonly property bool isExpanded: !screen.reorderMode && screen.expandedExercise === exercise
    readonly property real moveStep: Theme.layout.rowHeight + Theme.spacing.small
    property real moveStartY: 0

    function playMove(dir) {
        moveStartY = -dir * moveStep
        moveAnim.restart()
    }

    function setsSummary() {
        if (!exercise || !exercise.sets || exercise.sets.length === 0)
            return ""

        var values = []
        for (var i = 0; i < exercise.sets.length; ++i) {
            var set = exercise.sets[i]
            var loaded = set.secondaryAdjustable && parseFloat(set.secondaryText) !== 0
            var text = loaded ? set.secondaryText : set.primaryText
            if (text.length > 0 && values.indexOf(text) === -1)
                values.push(text)
        }

        var count = exercise.sets.length
        var label = Plural.counted(count, qsTr("set"), qsTr("sets"))
        if (values.length === 0)
            return label
        if (values.length === 1)
            return label + " · " + values[0]

        var unit = values[0].replace(/^[\d.,\s+-]+/, "")
        var min = Number.POSITIVE_INFINITY
        var max = Number.NEGATIVE_INFINITY
        for (var j = 0; j < values.length; ++j) {
            var value = parseFloat(values[j])
            if (isNaN(value) || values[j].replace(/^[\d.,\s+-]+/, "") !== unit)
                return label
            min = Math.min(min, value)
            max = Math.max(max, value)
        }
        return label + " · " + min + "-" + max + " " + unit
    }

    transform: Translate { id: moveTranslate; y: 0 }

    SequentialAnimation {
        id: moveAnim
        PropertyAction { target: moveTranslate; property: "y"; value: exerciseDelegate.moveStartY }
        NumberAnimation {
            target: moveTranslate
            property: "y"
            to: 0
            duration: 220
            easing.type: Easing.OutCubic
        }
    }

    Connections {
        target: exerciseDelegate.screen
        function onExerciseMoved(ex, dir) {
            if (ex === exerciseDelegate.exercise)
                exerciseDelegate.playMove(dir)
        }
    }

    Rectangle {
        id: header
        width: exerciseDelegate.width
        height: exerciseDelegate.isExpanded ? Theme.layout.listItemHeight - Theme.spacing.small
                                            : Theme.layout.rowHeight

        radius: Theme.radius.medium
        color: exerciseDelegate.isExpanded ? "transparent" : Theme.colors.surfaceMuted

        border.color: exerciseDelegate.isCurrent && !exerciseDelegate.isExpanded
                      ? Theme.colors.primaryBorder
                      : Theme.colors.borderSubtle
        border.width: exerciseDelegate.isExpanded ? 0 : Theme.border.thin

        Behavior on color { ColorAnimation { duration: 200 } }

        Rectangle {
            id: moveFlash
            anchors.fill: parent
            radius: parent.radius
            color: Theme.colors.primary
            opacity: 0

            SequentialAnimation {
                id: flashAnim
                running: moveAnim.running
                NumberAnimation { target: moveFlash; property: "opacity"; to: 0.25; duration: 110 }
                NumberAnimation { target: moveFlash; property: "opacity"; to: 0; duration: 220 }
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: exerciseDelegate.isExpanded ? 0 : Theme.padding.large
            anchors.rightMargin: exerciseDelegate.isExpanded ? 0 : Theme.padding.large
            spacing: Theme.spacing.medium

            Text {
                text: exercise.name

                Layout.fillWidth: true
                Layout.maximumWidth: implicitWidth + 1

                color: exerciseDelegate.isExpanded ? Theme.colors.textPrimary : Theme.colors.textMuted
                font.pixelSize: exerciseDelegate.isExpanded ? Theme.fontSize.medium : Theme.fontSize.normal
                font.bold: exerciseDelegate.isExpanded

                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            ThemedButton {
                objectName: "exerciseInfoButton" + exerciseDelegate.exerciseIndex
                iconSource: Theme.icons.info
                circular: true
                buttonSize: Theme.button.badge
                buttonStyle: Theme.button.ghost
                z: 1
                visible: exerciseDelegate.isExpanded
                Layout.alignment: Qt.AlignVCenter

                onClicked: exerciseDelegate.showExerciseInfo(exercise)
            }

            Item { Layout.fillWidth: true }

            Text {
                text: exerciseDelegate.setsSummary()
                color: Theme.colors.textDisabled
                font.pixelSize: Theme.fontSize.small
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                visible: !exerciseDelegate.isExpanded && !screen.reorderMode
                Layout.alignment: Qt.AlignVCenter
                Layout.maximumWidth: implicitWidth + 1
            }

            ThemedIcon {
                svgSource: Theme.icons.expand
                color: Theme.colors.textFaint
                width: Theme.icon.small
                height: Theme.icon.small
                visible: !exerciseDelegate.isExpanded && !screen.reorderMode && !exercise.completed
                Layout.alignment: Qt.AlignVCenter
            }

            Rectangle {
                width: Theme.button.badge.size
                height: Theme.button.badge.size
                radius: width / 2
                color: Theme.colors.successSurface
                visible: !exerciseDelegate.isExpanded && !screen.reorderMode && exercise.completed
                Layout.alignment: Qt.AlignVCenter

                ThemedIcon {
                    anchors.centerIn: parent
                    width: parent.width * 0.6
                    height: width
                    svgSource: Theme.icons.check
                    color: Theme.colors.textPrimary
                }
            }

            RowLayout {
                spacing: Theme.spacing.xSmall
                Layout.alignment: Qt.AlignVCenter
                visible: screen.reorderMode
                z: 1

                ThemedButton {
                    objectName: "activeMoveExerciseUpButton" + exerciseDelegate.exerciseIndex
                    iconSource: Theme.icons.moveUp
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.subtle
                    enabled: exerciseDelegate.exerciseIndex > 0
                    onClicked: exerciseDelegate.screen.requestMove(
                                   exercise, exerciseDelegate.exerciseIndex, -1)
                }

                ThemedButton {
                    objectName: "activeMoveExerciseDownButton" + exerciseDelegate.exerciseIndex
                    iconSource: Theme.icons.moveDown
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.subtle
                    enabled: exerciseDelegate.exerciseIndex < exerciseDelegate.exerciseCount - 1
                    onClicked: exerciseDelegate.screen.requestMove(
                                   exercise, exerciseDelegate.exerciseIndex, 1)
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            z: -1
            enabled: !screen.reorderMode
            onClicked: exerciseDelegate.screen.expandedExercise = exercise
        }
    }

    Text {
        objectName: "previousPerformanceText" + exerciseDelegate.exerciseIndex
        width: exerciseDelegate.width
        visible: exerciseDelegate.isExpanded && exerciseDelegate.hasPreviousSession
        text: qsTr("Last time") + " · " + Qt.formatDate(exercise.previousDate, "d MMM yyyy")
        color: Theme.colors.textMuted
        font.pixelSize: Theme.fontSize.small
        elide: Text.ElideRight
    }

    Column {
        id: setsColumn
        width: exerciseDelegate.width
        spacing: Theme.spacing.small
        property real revealProgress: exerciseDelegate.isExpanded ? 1 : 0

        height: implicitHeight * revealProgress
        visible: revealProgress > 0
        clip: true

        Behavior on revealProgress { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }

        Repeater {
            model: exercise.sets

            delegate: ActiveWorkoutSetItem {
                objectName: "activeWorkoutSetItem" + exerciseDelegate.exerciseIndex + "_" + index
                width: setsColumn.width
                number: index + 1
                setData: modelData
                previousShown: exercise.previousSetTexts.length > 0
                previousText: index < exercise.previousSetTexts.length
                              ? exercise.previousSetTexts[index]
                              : ""
            }
        }

        ThemedButton {
            objectName: "activeWorkoutAddSetButton" + exerciseDelegate.exerciseIndex
            width: setsColumn.width
            text: qsTr("Add set")
            iconSource: Theme.icons.addSet
            buttonSize: Theme.button.small
            buttonStyle: Theme.button.sunken
            visible: !screen.reorderMode
            onClicked: ActiveWorkoutViewModel.addSet(exercise)
        }
    }

    Item {
        width: exerciseDelegate.width
        height: Theme.spacing.small
        visible: exerciseDelegate.isExpanded
    }
}
