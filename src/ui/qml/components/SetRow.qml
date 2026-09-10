import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner
import Themed.Components

Rectangle {
    id: root

    property int number: 0
    property string previousText: ""
    property bool previousShown: false
    property string primaryText: ""
    property string secondaryText: ""
    property string metric: "reps"
    property string loadType: "external"

    property bool current: false
    property bool completed: false
    property bool expanded: false

    property bool completable: true
    property bool editable: true
    property bool expandable: true
    property bool secondaryAdjustable: true

    readonly property bool revealable: root.editable && root.expandable

    property string primaryLabel: root.metric === "duration" ? qsTr("TIME")
                                : root.metric === "distance" ? qsTr("DIST")
                                                             : qsTr("REPS")

    property string secondaryLabel: root.metric === "distance" ? qsTr("TIME")
                                  : root.loadType === "added" ? qsTr("+ KG")
                                  : root.loadType === "assisted" ? qsTr("- KG")
                                                                 : qsTr("KG")

    readonly property string namePrefix: root.objectName.length > 0 ? root.objectName + "_" : ""

    signal completionToggled()
    signal primaryAdjusted(int direction)
    signal secondaryAdjusted(int direction)
    signal duplicateRequested()
    signal removeRequested()

    readonly property int rowHeight: root.current ? Theme.layout.activeRowHeight
                                                  : Theme.layout.listItemHeight
    readonly property color valueColor: root.current ? Theme.colors.textPrimary
                                      : root.completed ? Theme.colors.textDisabled
                                                       : Theme.colors.textSecondary

    height: root.expanded ? root.rowHeight + editor.height + Theme.padding.small
                          : root.rowHeight
    radius: Theme.radius.medium
    clip: true

    color: root.current ? Theme.colors.surfaceAccent
         : root.completed ? Theme.colors.surfaceSunken
                          : Theme.colors.surface
    border.width: root.current ? Theme.border.medium : Theme.border.thin
    border.color: root.current ? Theme.colors.primary
                : root.completed ? Theme.colors.borderSubtle
                                 : Theme.colors.border

    Behavior on color { ColorAnimation { duration: 200 } }
    Behavior on border.color { ColorAnimation { duration: 200 } }
    Behavior on height { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }

    HoldToRevealArea {
        anchors.fill: parent
        enabled: root.editable
        onHeld: root.expanded = !root.expanded
        onTapped: {
            if (root.revealable)
                root.expanded = !root.expanded
        }
    }

    Column {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            width: parent.width
            height: root.rowHeight
            spacing: 0

            Text {
                text: root.number
                font.pixelSize: Theme.fontSize.small
                font.bold: root.current
                color: root.current ? Theme.colors.textPrimary : Theme.colors.textFaint
                Layout.preferredWidth: Theme.setRow.numberWidth
                Layout.leftMargin: Theme.padding.medium
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                objectName: root.namePrefix + "previousValueText"
                text: root.previousText
                font.pixelSize: Theme.setRow.previousFontSize
                color: Theme.colors.textFaint
                horizontalAlignment: Text.AlignLeft
                elide: Text.ElideRight
                Layout.preferredWidth: root.previousShown ? Theme.setRow.previousWidth : 0
                Layout.minimumWidth: Layout.preferredWidth
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                objectName: root.namePrefix + "primaryValueText"
                text: root.primaryText
                font.pixelSize: root.current ? Theme.setRow.valueCurrentFontSize
                                             : Theme.setRow.valueFontSize
                font.bold: root.current
                color: root.valueColor
                elide: Text.ElideRight
                Layout.preferredWidth: Theme.setRow.primaryWidth
                Layout.minimumWidth: Theme.setRow.primaryWidth
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                objectName: root.namePrefix + "secondaryValueText"
                text: root.secondaryText
                font.pixelSize: root.current ? Theme.setRow.valueCurrentFontSize
                                             : Theme.setRow.valueFontSize
                font.bold: root.current
                color: root.valueColor
                elide: Text.ElideRight
                Layout.fillWidth: true
                Layout.minimumWidth: Theme.setRow.valueMinWidth
                Layout.alignment: Qt.AlignVCenter
            }

            ThemedButton {
                objectName: root.namePrefix + "expandSetButton"
                iconSource: root.expanded ? Theme.icons.chevronUp : Theme.icons.chevronDown
                iconColor: root.expanded ? Theme.colors.primary : Theme.colors.textFaint
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.ghost
                visible: root.revealable
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: Theme.spacing.small
                onClicked: root.expanded = !root.expanded
                ToolTip.visible: hovered
                ToolTip.text: root.expanded ? qsTr("Hide set editor") : qsTr("Edit this set")
                ToolTip.delay: 500
            }

            CompletionDot {
                id: dot
                objectName: root.namePrefix + "completionDot"
                size: root.current ? Theme.layout.indicatorSizeLarge : Theme.layout.indicatorSize
                completed: root.completed
                highlighted: root.current
                interactive: root.completable
                visible: root.completable
                Layout.preferredWidth: dot.size
                Layout.preferredHeight: dot.size
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: Theme.padding.medium
                onToggled: root.completionToggled()
            }
        }

        Column {
            id: editor
            width: parent.width
            spacing: Theme.spacing.small
            visible: root.expanded

            Item {
                width: parent.width
                height: Theme.padding.small

                Rectangle {
                    x: Theme.padding.large
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - Theme.padding.large * 2
                    height: Theme.border.thin
                    color: Theme.colors.divider
                }
            }

            RowLayout {
                x: Theme.padding.large
                width: parent.width - Theme.padding.large * 2
                height: Theme.setRow.stepperHeight
                spacing: Theme.spacing.small

                StepperField {
                    label: root.primaryLabel
                    decrementName: root.namePrefix + "primaryDecrementButton"
                    incrementName: root.namePrefix + "primaryIncrementButton"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onDecremented: root.primaryAdjusted(-1)
                    onIncremented: root.primaryAdjusted(1)
                }

                StepperField {
                    label: root.secondaryLabel
                    decrementName: root.namePrefix + "secondaryDecrementButton"
                    incrementName: root.namePrefix + "secondaryIncrementButton"
                    visible: root.secondaryAdjustable
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onDecremented: root.secondaryAdjusted(-1)
                    onIncremented: root.secondaryAdjusted(1)
                }
            }

            RowLayout {
                x: Theme.padding.large
                width: parent.width - Theme.padding.large * 2
                height: Theme.setRow.actionHeight
                spacing: Theme.spacing.small

                ThemedButton {
                    objectName: root.namePrefix + "duplicateSetButton"
                    text: qsTr("Duplicate set")
                    iconSource: Theme.icons.addSet
                    buttonSize: Theme.button.small
                    buttonStyle: Theme.button.sunken
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onClicked: root.duplicateRequested()
                }

                ThemedButton {
                    objectName: root.namePrefix + "removeSetButton"
                    iconSource: Theme.icons.removeSet
                    buttonSize: Theme.button.small
                    buttonStyle: Theme.button.dangerSubtle
                    Layout.preferredWidth: Theme.setRow.removeWidth
                    Layout.fillHeight: true
                    onClicked: root.removeRequested()
                }
            }
        }
    }
}
