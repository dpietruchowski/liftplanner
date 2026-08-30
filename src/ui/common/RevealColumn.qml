import QtQuick
import QtQuick.Layouts
import Themed.Components

Item {
    id: root

    property bool expanded: true
    property int spacing: Theme.spacing.medium
    default property alias items: column.data

    readonly property real revealTarget: expanded ? column.implicitHeight : 0
    property real revealHeight: revealTarget

    Layout.fillWidth: true
    Layout.preferredHeight: revealHeight
    visible: revealHeight > 0
    clip: true

    onRevealTargetChanged: revealHeight = revealTarget

    Behavior on revealHeight {
        NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
    }

    ColumnLayout {
        id: column
        width: root.width
        spacing: root.spacing
    }
}
