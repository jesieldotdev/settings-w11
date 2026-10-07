// Cartão que expande, como "Escolher onde reproduzir o som": cabeçalho com
// ícone, título, descrição e o valor atual; abaixo, as linhas de conteúdo.
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Card {
    id: card
    property string icon
    property string title
    property string subtitle
    property string value
    property bool expanded: true
    default property alias rows: body.data

    Layout.fillWidth: true
    implicitHeight: header.height + (expanded ? body.implicitHeight : 0)
    clip: true
    Behavior on implicitHeight { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }

    Item {
        id: header
        width: parent.width
        height: 68

        MouseArea {
            id: headerMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: card.expanded = !card.expanded
            onContainsMouseChanged: card.hovered = containsMouse
        }

        Kirigami.Icon {
            x: 20
            anchors.verticalCenter: parent.verticalCenter
            width: 20
            height: 20
            source: card.icon
        }
        ColumnLayout {
            x: 56
            width: parent.width - x - valueLabel.width - 64
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
        QQC2.Label {
            id: valueLabel
            anchors.right: chevron.left
            anchors.rightMargin: 14
            anchors.verticalCenter: parent.verticalCenter
            width: Math.min(implicitWidth, 260)
            elide: Text.ElideRight
            text: card.value
            opacity: 0.8
        }
        Chevron {
            id: chevron
            anchors.right: parent.right
            anchors.rightMargin: 22
            anchors.verticalCenter: parent.verticalCenter
            kind: card.expanded ? "up" : "down"
        }
    }

    ColumnLayout {
        id: body
        y: header.height
        width: parent.width
        spacing: 0
    }
}
