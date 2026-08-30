import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: setRect
    objectName: "activeWorkoutSetItem" + index
    property var setData
    property bool actionsVisible: false

    readonly property bool isCurrent: ActiveWorkoutViewModel.currentSet === setData
    readonly property bool isCompleted: setData.completed
    readonly property int rowHeight: isCurrent ? Theme.layout.activeRowHeight : Theme.layout.listItemHeight

    readonly property int numberColumnWidth: 24
    readonly property int primaryColumnWidth: 66
    readonly property int columnPadding: Theme.padding.large

    readonly property string primaryLabel: {
        if (!setData)
            return ""
        if (setData.metric === "duration")
            return qsTr("TIME")
        if (setData.metric === "distance")
            return qsTr("DIST")
        return qsTr("REPS")
    }

    readonly property string secondaryLabel: {
        if (!setData)
            return ""
        if (setData.metric === "distance")
            return qsTr("TIME")
        if (setData.loadType === "added")
            return qsTr("+ KG")
        if (setData.loadType === "assisted")
            return qsTr("- KG")
        return qsTr("KG")
    }

    component SetStepper: Rectangle {
        id: stepper

        property string label
        property string decrementName
        property string incrementName

        signal decremented()
        signal incremented()

        radius: Theme.radius.medium
        color: Theme.colors.surfaceMuted
        border.width: Theme.border.thin
        border.color: Theme.colors.border

        RowLayout {
            anchors.fill: parent
            anchors.margins: Theme.padding.xSmall
            spacing: Theme.spacing.xSmall

            ThemedButton {
                objectName: stepper.decrementName
                iconSource: Theme.icons.minus
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.subtle
                onClicked: stepper.decremented()
            }

            Text {
                Layout.fillWidth: true
                text: stepper.label
                color: Theme.colors.textMuted
                font.pixelSize: Theme.setRow.labelSize
                font.letterSpacing: Theme.setRow.labelSpacing
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            ThemedButton {
                objectName: stepper.incrementName
                iconSource: Theme.icons.plus
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.subtle
                onClicked: stepper.incremented()
            }
        }
    }

    width: setsColumn.width
    height: actionsVisible ? rowHeight + actionsColumn.height + Theme.padding.small : rowHeight
    radius: Theme.radius.medium
    color: isCurrent ? Theme.colors.surfaceAccent
                     : isCompleted ? Theme.colors.surfaceSunken
                                   : Theme.colors.surface
    border.width: isCurrent ? Theme.border.medium : Theme.border.thin
    border.color: isCurrent ? Theme.colors.primary
                            : isCompleted ? Theme.colors.borderSubtle
                                          : Theme.colors.border

    Behavior on color { ColorAnimation { duration: 200 } }
    Behavior on border.color { ColorAnimation { duration: 200 } }
    Behavior on height { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }

    clip: true

    MouseArea {
        id: rowMouse
        anchors.fill: parent
        onPressAndHold: setRect.actionsVisible = !setRect.actionsVisible
        onPressed: holdAnim.restart()
        onReleased: { holdAnim.stop(); holdProgress.width = 0 }
        onCanceled: { holdAnim.stop(); holdProgress.width = 0 }
    }

    NumberAnimation {
        id: holdAnim
        target: holdProgress
        property: "width"
        from: 0
        to: setRect.width
        duration: Application.styleHints.mousePressAndHoldInterval
    }

    Rectangle {
        id: holdProgress
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        height: Theme.border.medium
        width: 0
        radius: height / 2
        color: Theme.colors.primary
        visible: width > 0
    }

    Column {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            width: parent.width
            height: setRect.rowHeight
            spacing: 0

            Text {
                text: index + 1
                font.pixelSize: Theme.fontSize.small
                font.bold: setRect.isCurrent
                color: setRect.isCurrent ? Theme.colors.textPrimary : Theme.colors.textFaint
                Layout.preferredWidth: setRect.numberColumnWidth
                Layout.leftMargin: setRect.columnPadding
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                text: setData.primaryText
                font.pixelSize: setRect.isCurrent ? Theme.fontSize.medium : Theme.fontSize.normal
                font.bold: setRect.isCurrent
                color: setRect.isCurrent ? Theme.colors.textPrimary
                                         : setRect.isCompleted ? Theme.colors.textDisabled
                                                               : Theme.colors.textSecondary
                elide: Text.ElideRight
                Layout.preferredWidth: setRect.primaryColumnWidth
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                text: setData.secondaryText
                font.pixelSize: setRect.isCurrent ? Theme.fontSize.medium : Theme.fontSize.normal
                font.bold: setRect.isCurrent
                color: setRect.isCurrent ? Theme.colors.textPrimary
                                         : setRect.isCompleted ? Theme.colors.textDisabled
                                                               : Theme.colors.textSecondary
                elide: Text.ElideRight
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
            }

            Rectangle {
                id: indicator
                property int size: setRect.isCurrent ? Theme.layout.indicatorSizeLarge : Theme.layout.indicatorSize

                Layout.preferredWidth: size
                Layout.preferredHeight: size
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: setRect.columnPadding

                radius: width / 2
                color: setRect.isCompleted ? Theme.colors.successSurface : "transparent"
                border.color: setRect.isCompleted ? Theme.colors.successSurface
                                                  : setRect.isCurrent ? Theme.colors.primary
                                                                      : Theme.colors.borderStrong
                border.width: Theme.border.medium

                Behavior on color { ColorAnimation { duration: 200 } }
                Behavior on border.color { ColorAnimation { duration: 200 } }

                ThemedIcon {
                    anchors.centerIn: parent
                    width: parent.width * 0.55
                    height: width
                    visible: setRect.isCompleted
                    svgSource: Theme.icons.check
                    color: Theme.colors.textPrimary
                }

                SequentialAnimation {
                    id: pulseAnim
                    NumberAnimation { target: indicator; property: "scale"; to: 1.3; duration: 110; easing.type: Easing.OutQuad }
                    NumberAnimation { target: indicator; property: "scale"; to: 1.0; duration: 160; easing.type: Easing.OutBack }
                }

                Connections {
                    target: setData
                    function onCompletedChanged() {
                        if (setData.completed)
                            pulseAnim.restart()
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: ActiveWorkoutViewModel.toggleSetCompleted(setData)
                }
            }
        }

        Column {
            id: actionsColumn
            width: parent.width
            spacing: Theme.spacing.small
            visible: setRect.actionsVisible

            Item {
                width: parent.width
                height: Theme.padding.small

                Rectangle {
                    x: setRect.columnPadding
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - setRect.columnPadding * 2
                    height: Theme.border.thin
                    color: Theme.colors.divider
                }
            }

            RowLayout {
                x: setRect.columnPadding
                width: parent.width - setRect.columnPadding * 2
                height: Theme.setRow.stepperHeight
                spacing: Theme.spacing.small

                SetStepper {
                    label: setRect.primaryLabel
                    decrementName: "primaryDecrementButton"
                    incrementName: "primaryIncrementButton"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onDecremented: ActiveWorkoutViewModel.adjustSetPrimary(setData, -1)
                    onIncremented: ActiveWorkoutViewModel.adjustSetPrimary(setData, 1)
                }

                SetStepper {
                    label: setRect.secondaryLabel
                    decrementName: "secondaryDecrementButton"
                    incrementName: "secondaryIncrementButton"
                    visible: setData.secondaryAdjustable
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onDecremented: ActiveWorkoutViewModel.adjustSetSecondary(setData, -1)
                    onIncremented: ActiveWorkoutViewModel.adjustSetSecondary(setData, 1)
                }
            }

            RowLayout {
                x: setRect.columnPadding
                width: parent.width - setRect.columnPadding * 2
                height: Theme.setRow.actionHeight
                spacing: Theme.spacing.small

                ThemedButton {
                    objectName: "duplicateSetButton"
                    text: qsTr("Duplicate set")
                    iconSource: Theme.icons.addSet
                    buttonSize: Theme.button.small
                    buttonStyle: Theme.button.sunken
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onClicked: ActiveWorkoutViewModel.duplicateSet(setData)
                }

                ThemedButton {
                    objectName: "removeSetButton"
                    iconSource: Theme.icons.removeSet
                    buttonSize: Theme.button.small
                    buttonStyle: Theme.button.dangerSubtle
                    Layout.preferredWidth: Theme.setRow.removeWidth
                    Layout.fillHeight: true
                    onClicked: ActiveWorkoutViewModel.removeSet(setData)
                }
            }
        }
    }
}
