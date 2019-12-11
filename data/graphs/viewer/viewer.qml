import QtQuick 2.6
import QtQuick.Controls 1.5
import QtQuick.Dialogs 1.2

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: qsTr("Graph Viewer Toy")

    MainForm {
        anchors.fill: parent

        period.model: ["homogeneous", "heterogeneous"]
        utili.model: ["light", "medium"]
        cs.model: ["short", "medium", "long"]
        nres.model: ["1", "2", "4", "8"]
        pacc.model: ["0.1", "0.25", "0.5"]

        image.source: "file://" + location.text + "/" + utili.currentText + "_" + period.currentText + "_" + cs.currentText + "_" + nres.currentText + "_" + pacc.currentText + ".png"
    }


}
