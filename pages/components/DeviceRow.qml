// Linha de dispositivo dentro de um cartão: bolinha de seleção, nome e detalhe
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: row
    property string title
    property string detail
    property bool selected: false
    signal clicked

    Layout.fillWidth: true
    implicitHeight: 58

    Rectangle { width: parent.width; height: 1; color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.07) }
    Rectangle {
        anchors.fill: parent
        anchors.topMargin: 1
        color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, mouse.containsMouse ? 0.04 : 0)
    }

    // bolinha: cheia na cor de destaque quando é o dispositivo em uso
    Rectangle {
        x: 20
        anchors.verticalCenter: parent.verticalCenter
        width: 20
        height: 20
        radius: 10
        color: row.selected ? Kirigami.Theme.highlightColor : "transparent"
        border.width: row.selected ? 0 : 1
        border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.6)
        Rectangle {
            visible: row.selected
            anchors.centerIn: parent
            width: 8
            height: 8
            radius: 4
            color: Kirigami.Theme.backgroundColor
        }
    }
    ColumnLayout {
        x: 56
        width: parent.width - x - 20
        anchors.verticalCenter: parent.verticalCenter
        spacing: 0
        QQC2.Label { text: row.title; font.weight: Font.DemiBold; elide: Text.ElideRight; Layout.fillWidth: true }
        QQC2.Label {
            text: row.detail
            visible: text.length > 0
            opacity: 0.7
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
    }
    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: row.clicked()
    }
}
