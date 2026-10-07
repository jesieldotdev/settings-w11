/*
 * Sistema › Som, como no Windows 11: Saída, Entrada e Avançado em cartões.
 * Dados do PulseAudio/PipeWire pelo módulo QML do plasma-pa.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.private.volume as Vol

import "components"

Page {
    id: page

    readonly property var sink: Vol.PreferredDevice.sink
    readonly property var source: Vol.PreferredDevice.source
    readonly property int maxVolume: 65536 // 100%

    // o canal "Sons do sistema" do plasma-w11 não é um dispositivo de verdade
    readonly property string systemSoundsSink: "input.loopback.sink.role.notification"

    Vol.PulseObjectFilterModel {
        id: sinks
        sourceModel: Vol.SinkModel {}
        filterOutInactiveDevices: true
        filters: [{ role: "Name", value: name => name !== page.systemSoundsSink }]
    }
    Vol.PulseObjectFilterModel {
        id: sources
        sourceModel: Vol.SourceModel {}
        filterOutInactiveDevices: true
    }
    Vol.PulseObjectFilterModel {
        id: apps
        sourceModel: Vol.SinkInputModel {}
        filters: [
            { role: "VirtualStream", value: false },
            { role: "Name", value: name => name.indexOf("speech-dispatcher") !== 0 }
        ]
    }
    Vol.VolumeFeedback { id: feedback }

    // ── Saída ───────────────────────────────────────────────────────────
    SectionTitle { text: "Saída" }

    ExpanderCard {
        icon: "audio-speakers-symbolic"
        title: "Escolher onde reproduzir o som"
        subtitle: "Os aplicativos podem ter suas próprias configurações"
        value: page.sink ? page.sink.description : ""

        Repeater {
            model: sinks
            delegate: DeviceRow {
                required property var model
                title: model.Description
                detail: model.PulseObject && model.PulseObject.ports.length > 0
                        ? model.PulseObject.ports[model.PulseObject.activePortIndex].description : ""
                selected: model.PulseObject && model.PulseObject.default
                onClicked: model.PulseObject.default = true
            }
        }
        ActionRow {
            text: "Emparelhar um novo dispositivo de saída"
            button: "Adicionar dispositivo"
            onClicked: settings.run("bluedevil-wizard")
        }
    }

    VolumeCard {
        title: "Volume"
        device: page.sink
        maxVolume: page.maxVolume
        onReleased: if (page.sink) feedback.play(page.sink.index)
    }

    // ── Entrada ─────────────────────────────────────────────────────────
    SectionTitle { text: "Entrada" }

    ExpanderCard {
        icon: "audio-input-microphone-symbolic"
        title: "Escolha um dispositivo para falar ou gravar"
        subtitle: "Os aplicativos podem ter suas próprias configurações"
        value: page.source ? page.source.description : "Nenhum"

        Repeater {
            model: sources
            delegate: DeviceRow {
                required property var model
                title: model.Description
                detail: model.PulseObject && model.PulseObject.ports.length > 0
                        ? model.PulseObject.ports[model.PulseObject.activePortIndex].description : ""
                selected: model.PulseObject && model.PulseObject.default
                onClicked: model.PulseObject.default = true
            }
        }
        ActionRow {
            text: "Emparelhar um novo dispositivo de entrada"
            button: "Adicionar dispositivo"
            onClicked: settings.run("bluedevil-wizard")
        }
    }

    VolumeCard {
        title: "Volume"
        device: page.source
        maxVolume: page.maxVolume
        microphone: true
    }

    // ── Avançado ────────────────────────────────────────────────────────
    SectionTitle { text: "Avançado" }

    ExpanderCard {
        icon: "view-media-equalizer"
        title: "Mixer de volume"
        subtitle: apps.count > 0 ? "Volume de cada aplicativo tocando agora" : "Nenhum aplicativo tocando som agora"

        Repeater {
            model: apps
            delegate: AppVolumeRow {
                required property var model
                title: (model.Client && model.Client.name) || model.Name || "Aplicativo"
                iconName: model.IconName || "audio-volume-high"
                stream: model.PulseObject
                maxVolume: page.maxVolume
            }
        }
    }

    LinkCard {
        icon: "configure"
        title: "Mais configurações de som"
        subtitle: "Perfis das placas, portas, canais e mais"
        onClicked: settings.openRawModule("kcm_pulseaudio")
    }
}
