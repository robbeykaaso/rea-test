import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 1.4
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import QtQuick.Controls.Universal 2.3
import "qml/gui/Basic"
import "qml/gui/Pipe"
import "qml/gui/Pipe/TreeNodeView"
import Pipeline 1.0
import QSGBoard 1.0

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    property var view_cfg
    Universal.theme: Universal.Dark

    TWindow{
        id: flip
        caption: "flipView"
        content: Flip{
            id: flipview
            anchors.fill: parent
            front: Rectangle{
                anchors.fill: parent
                color: "red"
            }
            back: Rectangle{
                anchors.fill: parent
                color: "blue"
            }
        }
    }

}
