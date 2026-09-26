import QtQuick
import QtQuick.Layouts
import Themed.Components

Item {
    id: root

    property bool expanded: true
    property int spacing: Theme.spacing.medium
    default property alias items: column.data

    property real revealProgress: expanded ? 1 : 0

    Layout.fillWidth: true
    Layout.preferredHeight: column.implicitHeight * revealProgress
    visible: revealProgress > 0
    clip: true

    Behavior on revealProgress {
        NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
    }

    ColumnLayout {
        id: column
        width: root.width
        spacing: root.spacing
    }
}
