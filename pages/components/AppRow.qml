// Cartão de um aplicativo instalado: ícone, nome, detalhes, tamanho e o "⋯"
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Card {
    id: row
    property string title
    property string icon
    property string details
    property string sizeText
    property bool working: false
    signal openRequested
    signal uninstallRequested(Item anchor)

    Layout.fillWidth: true
    implicitHeight: 68

    Kirigami.Icon {
        x: 20
        anchors.verticalCenter: parent.verticalCenter
        width: 32
        height: 32
        source: row.icon || "application-x-executable"
    }
    ColumnLayout {
        x: 68
        width: parent.width - x - 140
        anchors.verticalCenter: parent.verticalCenter
        spacing: 1
        QQC2.Label { text: row.title; elide: Text.ElideRight; Layout.fillWidth: true }
        QQC2.Label {
            visible: !row.working
            text: row.details
            opacity: 0.65
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
        QQC2.Label {
            visible: row.working
            text: "Desinstalando…"
            opacity: 0.65
            font.pointSize: Kirigami.Theme.smallFont.pointSize
        }
        QQC2.ProgressBar {
            visible: row.working
            indeterminate: true
            Layout.fillWidth: true
            Layout.maximumWidth: 260
        }
    }
    QQC2.Label {
        anchors.right: more.left
        anchors.rightMargin: 14
        anchors.verticalCenter: parent.verticalCenter
        text: row.sizeText
        opacity: 0.8
    }
    QQC2.Button {
        id: more
        anchors.right: parent.right
        anchors.rightMargin: 14
        anchors.verticalCenter: parent.verticalCenter
        flat: true
        enabled: !row.working
        icon.name: "view-more-horizontal-symbolic"
        display: QQC2.AbstractButton.IconOnly
        QQC2.ToolTip.text: "Mais opções"
        QQC2.ToolTip.visible: hovered
        onClicked: menu.popup(more, 0, more.height)

        QQC2.Menu {
            id: menu
            QQC2.MenuItem {
                text: "Abrir"
                icon.name: "window-new"
                onTriggered: row.openRequested()
            }
            QQC2.MenuItem {
                text: "Desinstalar"
                icon.name: "edit-delete"
                onTriggered: row.uninstallRequested(more)
            }
        }
    }
}
