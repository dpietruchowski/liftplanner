import LiftPlanner

SetRow {
    id: root

    property var setData

    primaryText: root.setData ? root.setData.primaryText : ""
    secondaryText: root.setData ? root.setData.secondaryText : ""
    metric: root.setData ? root.setData.metric : "reps"
    loadType: root.setData ? root.setData.loadType : "external"

    current: ActiveWorkoutViewModel.currentSet === root.setData
    completed: root.setData ? root.setData.completed : false
    secondaryAdjustable: root.setData ? root.setData.secondaryAdjustable : false

    onExpandToggled: root.expanded = !root.expanded
    onCompletionToggled: ActiveWorkoutViewModel.toggleSetCompleted(root.setData)
    onPrimaryAdjusted: function(direction) {
        ActiveWorkoutViewModel.adjustSetPrimary(root.setData, direction)
    }
    onSecondaryAdjusted: function(direction) {
        ActiveWorkoutViewModel.adjustSetSecondary(root.setData, direction)
    }
    onDuplicateRequested: ActiveWorkoutViewModel.duplicateSet(root.setData)
    onRemoveRequested: ActiveWorkoutViewModel.removeSet(root.setData)
}
