import QtQuick
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root

    property var workout: null
    property string label: ""
    property bool highlighted: false
    property bool boxed: false
    property bool dotted: false
    property color accent: Theme.colors.primary
    property color accentText: Theme.colors.primaryLight
    property string nameObjectName: ""
    property string placeholder: ""
    property string nameOverride: ""

    readonly property var workoutDate: {
        if (!workout)
            return null
        if (workout.status === "Planned")
            return workout.plannedTime && workout.plannedTime.getTime() > 0
                 ? workout.plannedTime : null
        if (workout.endedTime && workout.endedTime.getTime() > 0)
            return workout.endedTime
        if (workout.startedTime && workout.startedTime.getTime() > 0)
            return workout.startedTime
        if (workout.plannedTime && workout.plannedTime.getTime() > 0)
            return workout.plannedTime
        return null
    }

    readonly property string exerciseText: {
        if (!workout || !workout.exercises || workout.exercises.length === 0)
            return ""
        var names = []
        for (var i = 0; i < workout.exercises.length; ++i)
            names.push(workout.exercises[i].name)
        return names.join(" · ")
    }

    readonly property int animationDuration: 260

    readonly property string nameText: root.nameOverride.length > 0
                                       ? root.nameOverride
                                       : (workout ? workout.name : root.placeholder)
    readonly property real targetGap: highlighted ? Theme.drum.gapLarge : Theme.drum.gap
    readonly property real targetPadding: boxed ? Theme.drum.paddingLarge : Theme.drum.padding
    readonly property real targetTextWidth: width - 2 * targetPadding - Theme.drum.dateWidth
                                            - 2 * targetGap
    readonly property bool nameNeedsTwoLines: nameMetrics.width > targetTextWidth
    readonly property bool stretchTexts: highlighted && nameNeedsTwoLines
    readonly property bool compactRow: height < Theme.drum.rowHeightLarge

    property real sidePadding: boxed ? Theme.drum.paddingLarge : Theme.drum.padding
    property real textInset: sidePadding + Theme.drum.dateWidth
                             + (highlighted ? Theme.drum.gapLarge : Theme.drum.gap)
    property real textInsetRight: sidePadding
                                  + (highlighted ? Theme.drum.gapLarge : Theme.drum.gap)
    property real borderWidth: boxed ? Theme.border.medium : 0
    property real nameSize: highlighted ? Theme.drum.nameSizeLarge : Theme.drum.nameSize
    property real labelSize: highlighted ? Theme.drum.labelSizeLarge : Theme.drum.labelSize
    property real dateSize: highlighted ? Theme.drum.dateSizeLarge : Theme.drum.dateSize

    Behavior on sidePadding { NumberAnimation { duration: root.animationDuration } }
    Behavior on textInset { NumberAnimation { duration: root.animationDuration } }
    Behavior on textInsetRight { NumberAnimation { duration: root.animationDuration } }
    Behavior on borderWidth { NumberAnimation { duration: root.animationDuration } }
    Behavior on nameSize { NumberAnimation { duration: root.animationDuration } }
    Behavior on labelSize { NumberAnimation { duration: root.animationDuration } }
    Behavior on dateSize { NumberAnimation { duration: root.animationDuration } }

    radius: Theme.radius.large
    color: boxed ? Theme.colors.surface : "transparent"
    border.width: borderWidth
    border.color: accent

    Behavior on color { ColorAnimation { duration: root.animationDuration } }

    TextMetrics {
        id: nameMetrics
        text: root.nameText
        font.pixelSize: root.highlighted ? Theme.drum.nameSizeLarge : Theme.drum.nameSize
        font.bold: true
    }

    Column {
        anchors.left: parent.left
        anchors.leftMargin: root.sidePadding
        anchors.verticalCenter: parent.verticalCenter
        width: Theme.drum.dateWidth
        visible: root.workoutDate !== null

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: root.workoutDate ? Qt.formatDate(root.workoutDate, "d") : ""
            color: root.highlighted ? Theme.colors.textPrimary : Theme.colors.textMuted
            font.pixelSize: Math.round(root.dateSize)
            font.bold: true

            Behavior on color { ColorAnimation { duration: root.animationDuration } }
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: root.workoutDate ? Qt.formatDate(root.workoutDate, "MMM").toUpperCase() : ""
            color: root.highlighted ? Theme.colors.textMuted : Theme.colors.textFaint
            font.pixelSize: Theme.drum.monthSize
            font.letterSpacing: Theme.drum.labelSpacing

            Behavior on color { ColorAnimation { duration: root.animationDuration } }
        }
    }

    ColumnLayout {
        id: texts
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: root.stretchTexts ? parent.bottom : undefined
        anchors.leftMargin: root.textInset
        anchors.rightMargin: root.textInsetRight
        anchors.topMargin: root.stretchTexts
                           ? Theme.drum.textSpacing
                           : Math.round((root.height - implicitHeight) / 2)
        anchors.bottomMargin: Theme.drum.textSpacing
        spacing: root.highlighted ? Theme.drum.textSpacingLarge : Theme.drum.textSpacing

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Theme.spacing.small
            visible: root.label.length > 0

            Rectangle {
                width: Theme.drum.dotSize
                height: Theme.drum.dotSize
                radius: width / 2
                color: root.accent
                visible: root.dotted
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                text: root.label.toUpperCase()
                color: root.dotted || root.boxed ? root.accentText : Theme.colors.textFaint
                font.pixelSize: Math.round(root.labelSize)
                font.letterSpacing: Theme.drum.labelSpacing
                Layout.alignment: Qt.AlignVCenter

                Behavior on color { ColorAnimation { duration: root.animationDuration } }
            }
        }

        Text {
            objectName: root.nameObjectName
            Layout.fillWidth: true
            Layout.fillHeight: root.stretchTexts
            text: root.nameText
            color: root.highlighted ? Theme.colors.textPrimary : Theme.colors.textSoft
            font.pixelSize: Math.round(root.nameSize)
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: root.stretchTexts ? Text.WordWrap : Text.NoWrap
            maximumLineCount: root.stretchTexts ? 2 : 1
            fontSizeMode: root.stretchTexts ? Text.Fit : Text.HorizontalFit
            minimumPixelSize: Math.round(root.nameSize * 0.55)
            elide: Text.ElideRight

            Behavior on color { ColorAnimation { duration: root.animationDuration } }
        }

        Text {
            Layout.fillWidth: true
            text: root.exerciseText
            color: root.highlighted ? Theme.colors.textMuted : Theme.colors.textFaint
            font.pixelSize: Theme.drum.exerciseSize
            horizontalAlignment: Text.AlignHCenter
            wrapMode: root.highlighted && !root.compactRow ? Text.WordWrap : Text.NoWrap
            maximumLineCount: root.highlighted && !root.compactRow ? 2 : 1
            elide: Text.ElideRight
            visible: root.exerciseText.length > 0 && !root.stretchTexts

            Behavior on color { ColorAnimation { duration: root.animationDuration } }
        }
    }
}
