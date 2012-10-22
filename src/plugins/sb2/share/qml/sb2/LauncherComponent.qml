import QtQuick 1.1
import "."

Rectangle {
    id: rootRect

    width: parent.width
    property real launcherItemHeight: parent.width
    height: launcherView.count * launcherItemHeight

    color: "transparent"

    ListView {
        id: launcherView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds

        model: SB2_launcherModel

        delegate: ActionButton {
            height: rootRect.launcherItemHeight
            width: rootRect.width

            actionIconURL: tabClassIcon

            onTriggered: SB2_launcherProxy.tabOpenRequested(tabClassID)
        }
    }
}
