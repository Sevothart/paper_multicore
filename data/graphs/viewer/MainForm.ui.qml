import QtQuick 2.6
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

Item {
    id: item1
    width: 800
    height: 600
    property alias location: location
    property alias image: image
    property alias period: period
    property alias utili: utili
    property alias cs: cs
    property alias pacc: pacc
    property alias nres: nres


    RowLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.fill: parent
        transformOrigin: Item.Center

        Rectangle {
            id: rectangle1
            width: 200
            height: 200
            color: "#ffffff"
            Layout.fillWidth: true
            Layout.fillHeight: true

            Image {
                id: image
                x: 27
                y: 163
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                source: "qrc:/qtquickplugin/images/template_image.png"
            }
        }

        ColumnLayout {
            id: columnLayout1
            width: 150
            height: 100

            TextField {
                id: location
                text: "/home/camargo/Seafile/LISHA/papers/in_preparation/sbesc2016-sync-lucas/data/graphs/output_png/"
                placeholderText: qsTr("Text Field")
            }

            Label {
                id: label1
                text: qsTr("Period")
            }

            ComboBox {
                id: period
            }

            Label {
                id: label2
                text: qsTr("Utilization")
            }

            ComboBox {
                id: utili
            }

            Label {
                id: label3
                text: qsTr("Critical Section")
            }

            ComboBox {
                id: cs
            }

            Label {
                id: label4
                text: qsTr("Num. Resources")
            }

            ComboBox {
                id: nres
            }

            Label {
                id: label5
                text: qsTr("Resource Acces Prob.")
            }

            ComboBox {
                id: pacc
            }









        }
    }
}
