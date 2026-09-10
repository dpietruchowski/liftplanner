import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root
    objectName: "screenHome"
    color: Theme.colors.background

    anchors.margins: Theme.padding.screen

    property var plannedWorkouts: []
    property var historyWorkouts: []

    function readWorkoutLists() {
        root.plannedWorkouts = PlannedWorkoutViewModel.workouts
        root.historyWorkouts = WorkoutHistoryViewModel.workouts
    }

    Component.onCompleted: root.readWorkoutLists()

    Connections {
        target: PlannedWorkoutViewModel
        function onWorkoutsChanged() { root.readWorkoutLists() }
    }

    Connections {
        target: WorkoutHistoryViewModel
        function onWorkoutsChanged() { root.readWorkoutLists() }
    }

    readonly property var drumEntries: {
        var list = []
        var planned = root.plannedWorkouts
        var current = ActiveWorkoutViewModel.currentWorkout
        var history = root.historyWorkouts
        list.push({ label: "without a plan", kind: "blank", itemName: "drumItemBlank",
                    workout: PlannedWorkoutViewModel.blankWorkout })
        if (current) {
            if (planned.length > 0)
                list.push({ label: "planned workout", kind: "planned", workout: planned[0] })
            list.push({ label: "current workout", kind: "current", workout: current })
        } else {
            if (planned.length > 1)
                list.push({ label: "next planned", kind: "upcoming", workout: planned[1] })
            if (planned.length > 0)
                list.push({ label: "planned workout", kind: "planned", workout: planned[0] })
        }
        list.push({ label: "last workout", kind: "last", placeholder: "Nothing done yet",
                    workout: history.length > 0 ? history[0] : null })
        return list
    }

    readonly property var selectedWorkout: {
        var index = workoutDrum.currentIndex
        if (index < 0 || index >= drumEntries.length)
            return null
        return drumEntries[index].workout
    }

    readonly property var startDecision: WorkoutStartPolicy.decide(
                                             selectedWorkout,
                                             ActiveWorkoutViewModel.currentWorkout)

    ColumnLayout {
        id: content
        objectName: "homeContent"
        width: parent.width - 2 * Theme.padding.screen
        height: Math.min(implicitHeight, parent.height - 2 * Theme.padding.screen)
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 0

        Text {
            objectName: "todayLabel"
            Layout.fillWidth: true
            Layout.minimumHeight: implicitHeight
            text: Qt.formatDate(new Date(), "dddd, d MMM").toUpperCase()
            color: Theme.colors.textMuted
            font.pixelSize: Theme.fontSize.xSmall
            font.letterSpacing: Theme.drum.labelSpacing
        }

        WeekActivityStrip {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: Theme.spacing.large
            Layout.minimumHeight: implicitHeight
            activity: WorkoutHistoryViewModel.weekActivity
            todayIndex: (new Date().getDay() + 6) % 7
        }

        Item {
            id: topExerciseRow
            objectName: "topExerciseRow"
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacing.xLarge
            Layout.minimumHeight: implicitHeight
            Layout.preferredHeight: implicitHeight

            readonly property int tileCount: WorkoutHistoryViewModel.topExercises.length
            readonly property real tileWidth: tileCount > 0
                ? (width - Theme.stat.spacing * (tileCount - 1)) / tileCount
                : 0

            implicitHeight: tileCount > 0 ? Theme.stat.height : 0

            Repeater {
                model: WorkoutHistoryViewModel.topExercises

                StatTile {
                    objectName: "topExerciseTile_" + modelData.name
                    x: index * (topExerciseRow.tileWidth + Theme.stat.spacing)
                    width: topExerciseRow.tileWidth
                    height: topExerciseRow.height
                    label: modelData.name
                    value: Math.round(modelData.oneRepMax)
                    unit: "kg"
                    badge: "PR"
                }
            }
        }

        Item {
            id: totalsRow
            objectName: "totalsRow"
            Layout.fillWidth: true
            Layout.topMargin: tileCount > 0 ? Theme.stat.spacing : 0
            Layout.minimumHeight: implicitHeight
            Layout.preferredHeight: implicitHeight

            readonly property int tileCount: WorkoutHistoryViewModel.recentTotals.length
            readonly property real tileWidth: tileCount > 0
                ? (width - Theme.stat.spacing * (tileCount - 1)) / tileCount
                : 0

            implicitHeight: tileCount > 0 ? Theme.stat.height : 0

            Repeater {
                model: WorkoutHistoryViewModel.recentTotals

                StatTile {
                    objectName: "totalsTile_" + modelData.label
                    x: index * (totalsRow.tileWidth + Theme.stat.spacing)
                    width: totalsRow.tileWidth
                    height: totalsRow.height
                    label: modelData.label
                    value: modelData.value
                }
            }
        }

        WorkoutDrum {
            id: workoutDrum
            objectName: "workoutDrum"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: Theme.spacing.xLarge
            Layout.minimumHeight: 0
            Layout.maximumHeight: implicitHeight
            entries: root.drumEntries
            defaultIndex: root.drumEntries.length > 1 ? root.drumEntries.length - 2 : 0
        }

        ThemedButton {
            objectName: "startWorkoutButton"
            Layout.fillWidth: true
            Layout.leftMargin: Theme.drum.cardInset
            Layout.rightMargin: Theme.drum.cardInset
            Layout.topMargin: Theme.spacing.xLarge
            Layout.minimumHeight: implicitHeight
            text: root.startDecision.label
            iconSource: Theme.icons.startWorkout
            iconSize: Theme.icon.large
            pill: true
            buttonStyle: Theme.button.primary
            buttonSize: Theme.button.wide
            onClicked: root.handleStartRequest()
        }
    }

    NotificationPopup {
        id: noPlannedPopup
        objectName: "noPlannedPopup"
        title: "Nothing to start here"
        text: "You can begin without a plan: an empty session opens right away and you add exercises as you go.\n\n" +
              "For a plan, head to the workouts tab (calendar icon) and tap the AI button — it copies a ready-made prompt to your clipboard. " +
              "Paste it into any AI assistant (ChatGPT, Gemini, etc.), describe your training goals, and let it generate a workout plan. " +
              "Once you receive the JSON, come back and tap the import button next to 'Planned'."
        type: Notification.Type.Info
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        okText: "Start empty"
        onAccepted: root.startBlankWorkout()
    }

    NotificationPopup {
        id: startWorkoutPopup
        objectName: "replaceWorkoutPopup"
        title: "Replace the running workout?"
        text: root.startDecision.confirmation
        type: Notification.Type.Warning
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        onAccepted: root.startWorkout(root.selectedWorkout)
    }

    NotificationPopup {
        id: cannotStartPopup
        objectName: "cannotStartPopup"
        type: Notification.Type.Info
        buttons: Notification.Button.Ok
    }

    function handleStartRequest() {
        var decision = root.startDecision
        if (decision.action === "missing") {
            noPlannedPopup.open()
            return
        }
        if (decision.action === "blocked") {
            cannotStartPopup.text = decision.message
            cannotStartPopup.open()
            return
        }
        if (decision.action === "resume") {
            showActiveWorkout()
            return
        }
        if (decision.action === "replace") {
            startWorkoutPopup.open()
            return
        }
        startWorkout(root.selectedWorkout)
    }

    function blankEntryIndex() {
        for (var i = 0; i < root.drumEntries.length; ++i) {
            if (root.drumEntries[i].kind === "blank")
                return i
        }
        return -1
    }

    function startBlankWorkout() {
        var index = root.blankEntryIndex()
        if (index >= 0)
            workoutDrum.focusEntry(index)

        var blank = PlannedWorkoutViewModel.blankWorkout
        if (!blank)
            return

        var decision = WorkoutStartPolicy.decide(blank, ActiveWorkoutViewModel.currentWorkout)
        if (decision.action === "replace") {
            startWorkoutPopup.open()
            return
        }
        root.startWorkout(blank)
    }

    function startWorkout(workout) {
        if (!workout)
            return
        ActiveWorkoutViewModel.startWorkout(workout)
        PlannedWorkoutViewModel.loadAll()
        showActiveWorkout()
    }

    function showActiveWorkout() {
        if (stackView.currentItem !== activeWorkoutScreen) {
            stackView.replace(activeWorkoutScreen)
        }
    }
}
