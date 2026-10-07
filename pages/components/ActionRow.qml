// Linha com texto apagado e um botão à direita ("Adicionar dispositivo")
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: row
    property string text
    property string button
    signal clicked

    Layout.fillWidth: true
    implicitHeight: 54

    Rectangle { width: parent.width; height: 1; color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.07) }
    QQC2.Label {
        x: 56
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - x - actionButton.width - 40
        text: row.text
        opacity: 0.7
        elide: Text.ElideRight
    }
    QQC2.Button {
        id: actionButton
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        text: row.button
        onClicked: row.clicked()
    }
}
