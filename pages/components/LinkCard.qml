// Cartão que leva a outro lugar (↗), como "Mais configurações de som"
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Card {
    id: card
    property string icon
    property string title
    property string subtitle
    signal clicked

    Layout.fillWidth: true
    implicitHeight: 68
    hovered: mouse.containsMouse
    pressed: mouse.pressed

    Kirigami.Icon {
        x: 20
        anchors.verticalCenter: parent.verticalCenter
        width: 20
        height: 20
        source: card.icon
    }
    ColumnLayout {
        x: 56
        width: parent.width - x - 56
        anchors.verticalCenter: parent.verticalCenter
        spacing: 0
        QQC2.Label { text: card.title; elide: Text.ElideRight; Layout.fillWidth: true }
        QQC2.Label {
            text: card.subtitle
            visible: text.length > 0
            opacity: 0.65
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
    }
    Chevron {
        anchors.right: parent.right
        anchors.rightMargin: 22
        anchors.verticalCenter: parent.verticalCenter
        kind: "external"
    }
    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: card.clicked()
    }
}
