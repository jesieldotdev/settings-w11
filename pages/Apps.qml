/*
 * Aplicativos › Aplicativos instalados, como no Windows 11: pesquisa,
 * ordenação, filtro e a lista; o "⋯" de cada app desinstala direto
 * (pacote do sistema, Flatpak, Snap, AppImage ou atalho), sem loja.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import SettingsW11

import "components"

Page {
    id: page

    AppsModel { id: apps }

    // ── pesquisa, ordenação e filtro ───────────────────────────────────
    QQC2.TextField {
        Layout.preferredWidth: 360
        Layout.topMargin: 4
        placeholderText: "Pesquisar aplicativos"
        onTextChanged: apps.filterText = text
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.topMargin: 10
        Layout.bottomMargin: 6
        spacing: 8

        QQC2.Label {
            text: apps.loading ? "Procurando aplicativos…"
                  : apps.count === 1 ? "1 aplicativo encontrado" : apps.count + " aplicativos encontrados"
            Layout.fillWidth: true
        }
        QQC2.Label { text: "Classificar por:"; opacity: 0.8 }
        QQC2.ComboBox {
            model: [
                { text: "Nome (A a Z)", value: "name" },
                { text: "Nome (Z a A)", value: "name-desc" },
                { text: "Tamanho (Grande para pequeno)", value: "size" },
                { text: "Data de instalação", value: "date" }
            ]
            textRole: "text"
            valueRole: "value"
            onActivated: apps.sortBy = currentValue
        }
        QQC2.Label { text: "Filtrar por:"; opacity: 0.8; Layout.leftMargin: 8 }
        QQC2.ComboBox {
            model: [
                { text: "Todas as fontes", value: "" },
                { text: "Sistema (RPM)", value: "rpm" },
                { text: "Flatpak", value: "flatpak" },
                { text: "Snap", value: "snap" },
                { text: "AppImage e atalhos", value: "outros" }
            ]
            textRole: "text"
            valueRole: "value"
            onActivated: apps.source = currentValue
        }
    }

    Card {
        id: notice
        property alias text: noticeLabel.text
        property bool ok: true
        visible: false
        Layout.fillWidth: true
        Layout.bottomMargin: 6
        implicitHeight: 48
        color: ok ? Qt.rgba(0.3, 0.7, 0.3, 0.15) : Qt.rgba(0.85, 0.3, 0.3, 0.18)
        QQC2.Label {
            id: noticeLabel
            x: 16
            width: parent.width - 32
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideRight
        }
        Timer { id: noticeTimer; interval: 6000; onTriggered: notice.visible = false }
    }

    QQC2.BusyIndicator {
        visible: apps.loading
        running: visible
        Layout.alignment: Qt.AlignHCenter
        Layout.topMargin: 40
    }

    // ── a lista ────────────────────────────────────────────────────────
    Repeater {
        model: apps
        delegate: AppRow {
            required property int index
            required property string name
            required property string iconName
            required property string kindName
            required property int kind
            required property string version
            required property string publisher
            required property var size
            required property var installed
            required property string desktop
            required property bool busy

            title: name
            icon: iconName
            details: {
                const parts = [];
                if (version) parts.push(version);
                if (publisher) parts.push(publisher);
                if (installed && !isNaN(installed)) parts.push(installed.toLocaleDateString(Qt.locale(), "dd/MM/yyyy"));
                parts.push(kindName);
                return parts.join("  |  ");
            }
            sizeText: size > 0 ? Qt.locale().formattedDataSize(size, 1, Locale.DataSizeSIFormat) : ""
            working: busy
            onOpenRequested: apps.launch(index)
            onUninstallRequested: (anchor) => confirm.ask(index, desktop, name, kind, anchor)
        }
    }

    // ── confirmação, como o balão do Windows ───────────────────────────
    QQC2.Popup {
        id: confirm
        property int row: -1
        property string desktop
        property string appName
        property int kind
        property bool checking: false
        property bool allowed: true
        property var also: []
        property string message

        function ask(row, desktop, name, kind, anchor) {
            confirm.row = row;
            confirm.desktop = desktop;
            confirm.appName = name;
            confirm.kind = kind;
            confirm.checking = true;
            confirm.allowed = true;
            confirm.also = [];
            confirm.message = "";
            const p = anchor.mapToItem(page, 0, anchor.height);
            confirm.x = Math.min(p.x + anchor.width - width, page.width - width - 12);
            confirm.y = p.y + 4;
            confirm.open();
            apps.planUninstall(row);
        }

        Connections {
            target: apps
            function onPlanReady(desktop, ok, also, message) {
                if (desktop !== confirm.desktop) return;
                confirm.checking = false;
                confirm.allowed = ok;
                confirm.also = also;
                confirm.message = message;
            }
        }

        width: 340
        padding: 16
        modal: false
        focus: true
        closePolicy: QQC2.Popup.CloseOnEscape | QQC2.Popup.CloseOnPressOutside

        background: Kirigami.ShadowedRectangle {
            radius: 8
            color: Qt.tint(Kirigami.Theme.backgroundColor, Qt.rgba(1, 1, 1, 0.04))
            border.width: 1
            border.color: Qt.rgba(0, 0, 0, 0.35)
            shadow.size: 18
            shadow.yOffset: 6
            shadow.color: Qt.rgba(0, 0, 0, 0.35)
        }

        contentItem: ColumnLayout {
            spacing: 12
            QQC2.Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: confirm.checking ? "Verificando o que será removido…"
                      : !confirm.allowed ? confirm.message
                      : confirm.kind >= 3 ? "O atalho de \"" + confirm.appName + "\" (e o arquivo do programa, se for um AppImage) vai para a lixeira."
                      : "Este aplicativo e as informações relacionadas serão desinstalados."
            }
            QQC2.Label {
                visible: !confirm.checking && confirm.also.length > 0
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                opacity: 0.75
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                text: (confirm.allowed ? "Também serão removidos: " : "Sairiam junto: ") + confirm.also.join(", ")
            }
            QQC2.ProgressBar {
                visible: confirm.checking
                indeterminate: true
                Layout.fillWidth: true
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                QQC2.Button {
                    text: confirm.allowed ? "Cancelar" : "OK"
                    onClicked: confirm.close()
                }
                QQC2.Button {
                    visible: confirm.allowed
                    enabled: !confirm.checking
                    highlighted: true
                    text: "Desinstalar"
                    onClicked: {
                        apps.uninstall(confirm.row);
                        confirm.close();
                    }
                }
            }
        }
    }

    // aviso no fim (deu certo / falhou)
    Connections {
        target: apps
        function onUninstallFinished(name, ok, message) {
            notice.text = ok ? "\"" + name + "\" foi desinstalado." : "Não foi possível desinstalar \"" + name + "\". " + message;
            notice.ok = ok;
            notice.visible = true;
            noticeTimer.restart();
        }
    }
}
