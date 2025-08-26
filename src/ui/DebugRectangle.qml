import QtQuick 2.15

Rectangle {
    id: root
    property Item target: null

    color: "transparent"
    border.color: "red"
    border.width: 2
    radius: 4
    z: 999

    x: target ? target.x : 0
    y: target ? target.y : 0
    width: target ? target.width : 0
    height: target ? target.height : 0
}
