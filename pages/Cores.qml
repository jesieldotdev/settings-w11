/*
 * Personalização › Cores, como no Windows 11: cor de destaque automática
 * (tirada do papel de parede) ou manual, com as 48 cores do Windows e a
 * opção de escolher qualquer outra. Vale para o painel, a bandeja, o menu
 * Iniciar e os apps (quem estiver aberto em widgets muda ao reabrir).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import "components"

Page {
    id: page

    // as cores de destaque do Windows, na ordem da grade dele (8 × 6)
    readonly property var windowsColors: [
        "#FFB900", "#FF8C00", "#F7630C", "#CA5010", "#DA3B01", "#EF6950", "#D13438", "#FF4343",
        "#E74856", "#E81123", "#EA005E", "#C30052", "#E3008C", "#BF0077", "#C239B3", "#9A0089",
        "#0078D7", "#0063B1", "#8E8CD8", "#6B69D6", "#8764B8", "#744DA9", "#B146C2", "#881798",
        "#0099BC", "#2D7D9A", "#00B7C3", "#038387", "#00B294", "#018574", "#00CC6A", "#10893E",
        "#7A7574", "#5D5A58", "#68768A", "#515C6B", "#567C73", "#486860", "#498205", "#107C10",
        "#767676", "#4C4A48", "#69797E", "#4A5459", "#647C64", "#525E54", "#847545", "#7E735F"
    ]

    property bool automatic: false
    property color accent: "#0078D7"

    function load() {
        automatic = settings.readConfig("kdeglobals", "General", "accentColorFromWallpaper") === "true";
        const rgb = settings.readConfig("kdeglobals", "General", "AccentColor").split(",");
        if (rgb.length === 3) {
            accent = Qt.rgba(rgb[0] / 255, rgb[1] / 255, rgb[2] / 255, 1);
        }
    }
    function hex(c) {
        const h = v => ("0" + Math.round(v * 255).toString(16)).slice(-2);
        return ("#" + h(c.r) + h(c.g) + h(c.b)).toUpperCase();
    }
    function setManual(color) {
        settings.writeConfig("kdeglobals", "General", "accentColorFromWallpaper", "false");
        settings.run("plasma-apply-colorscheme --accent-color " + hex(color));
        accent = color;
        automatic = false;
        reloadLater.restart();
    }
    function setAutomatic() {
        settings.writeConfig("kdeglobals", "General", "accentColorFromWallpaper", "true");
        // o serviço de cor de destaque do Plasma escuta este aviso e tira a cor do papel de parede
        settings.run("dbus-send --session --type=signal /KGlobalSettings org.kde.KGlobalSettings.notifyChange int32:0 int32:0");
        automatic = true;
        reloadLater.restart();
    }

    Component.onCompleted: load()
    Timer { id: reloadLater; interval: 2500; onTriggered: page.load() }

    // ── cor de destaque ────────────────────────────────────────────────
    Card {
        Layout.fillWidth: true
        Layout.topMargin: 4
        implicitHeight: header.height + body.implicitHeight

        Item {
            id: header
            width: parent.width
            height: 68
            Kirigami.Icon {
                x: 20
                anchors.verticalCenter: parent.verticalCenter
                width: 20
                height: 20
                source: "preferences-desktop-color"
            }
            ColumnLayout {
                x: 56
                width: parent.width - x - modeBox.width - 40
                anchors.verticalCenter: parent.verticalCenter
                spacing: 0
                QQC2.Label { text: "Cor de destaque"; Layout.fillWidth: true }
                QQC2.Label {
                    text: page.automatic ? "Tirada do papel de parede; muda junto quando ele mudar"
                                         : "Usada no painel, na bandeja, no menu Iniciar e nos apps"
                    opacity: 0.65
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
            QQC2.ComboBox {
                id: modeBox
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                model: ["Manual", "Automático"]
                currentIndex: page.automatic ? 1 : 0
                onActivated: index => index === 1 ? page.setAutomatic() : page.setManual(page.accent)
            }
        }

        ColumnLayout {
            id: body
            y: header.height
            width: parent.width
            spacing: 0

            Rectangle { Layout.fillWidth: true; height: 1; color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.07) }

            QQC2.Label {
                text: "Cores do Windows"
                Layout.leftMargin: 56
                Layout.topMargin: 14
                Layout.bottomMargin: 10
            }

            Grid {
                Layout.leftMargin: 56
                Layout.bottomMargin: 18
                columns: 8
                spacing: 4

                Repeater {
                    model: page.windowsColors
                    delegate: Rectangle {
                        required property string modelData
                        readonly property bool chosen: !page.automatic && page.hex(page.accent) === modelData
                        width: 40
                        height: 40
                        radius: 4
                        color: modelData
                        // escolhida: anel claro em volta e o visto no canto, como no Windows
                        border.width: chosen ? 2 : (swatchMouse.containsMouse ? 1 : 0)
                        border.color: Kirigami.Theme.textColor
                        scale: swatchMouse.pressed ? 0.94 : 1
                        Behavior on scale { NumberAnimation { duration: 90 } }

                        Rectangle {
                            visible: parent.chosen
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.margins: 4
                            width: 14
                            height: 14
                            radius: 7
                            color: Kirigami.Theme.textColor
                            Kirigami.Icon {
                                anchors.centerIn: parent
                                width: 10
                                height: 10
                                source: "checkmark"
                                color: Kirigami.Theme.backgroundColor
                                isMask: true
                            }
                        }
                        MouseArea {
                            id: swatchMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: page.setManual(parent.modelData)
                        }
                        QQC2.ToolTip.text: modelData
                        QQC2.ToolTip.visible: swatchMouse.containsMouse
                        QQC2.ToolTip.delay: 600
                    }
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.07) }

            Item {
                Layout.fillWidth: true
                implicitHeight: 58
                QQC2.Label {
                    x: 56
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Cores personalizadas"
                }
                Rectangle { // a cor atual
                    anchors.right: pick.left
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    width: 24
                    height: 24
                    radius: 4
                    color: page.accent
                    border.width: 1
                    border.color: Qt.rgba(1, 1, 1, 0.2)
                }
                QQC2.Button {
                    id: pick
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Exibir cores"
                    onClicked: {
                        colorDialog.selectedColor = page.accent;
                        colorDialog.open();
                    }
                }
            }
        }
    }

    LinkCard {
        icon: "preferences-desktop-theme-global"
        title: "Mais opções de cores"
        subtitle: "Esquemas de cores, cores das janelas e do texto"
        onClicked: settings.openRawModule("kcm_colors")
    }

    ColorDialog {
        id: colorDialog
        title: "Escolha uma cor de destaque"
        onAccepted: page.setManual(selectedColor)
    }
}
