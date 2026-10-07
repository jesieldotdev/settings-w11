// Cartão "Volume": título à esquerda; à direita, o alto-falante (mudo), o valor e o slider
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Card {
    id: card
    property string title
    property var device: null
    property int maxVolume: 65536
    property bool microphone: false
    signal released

    readonly property int percent: device ? Math.round((device.muted ? 0 : device.volume) / maxVolume * 100) : 0

    Layout.fillWidth: true
    implicitHeight: 62
    enabled: device !== null

    QQC2.Label {
        x: 20
        anchors.verticalCenter: parent.verticalCenter
        text: card.title
    }
    RowLayout {
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        Kirigami.Icon {
            Layout.preferredWidth: 20
            Layout.preferredHeight: 20
            source: card.microphone
                    ? (card.device && card.device.muted ? "microphone-sensitivity-muted-symbolic" : "audio-input-microphone-symbolic")
                    : (card.device && card.device.muted ? "audio-volume-muted-symbolic"
                       : card.percent > 66 ? "audio-volume-high-symbolic" : card.percent > 33 ? "audio-volume-medium-symbolic" : "audio-volume-low-symbolic")
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: if (card.device) card.device.muted = !card.device.muted
            }
        }
        QQC2.Label {
            Layout.preferredWidth: 30
            horizontalAlignment: Text.AlignRight
            text: card.percent
        }
        QQC2.Slider {
            Layout.preferredWidth: 170
            from: 0
            to: card.maxVolume
            value: card.device ? (card.device.muted ? 0 : card.device.volume) : 0
            onMoved: {
                card.device.volume = Math.round(value / card.maxVolume * 100) * card.maxVolume / 100;
                card.device.muted = value < card.maxVolume / 200;
            }
            onPressedChanged: if (!pressed) card.released()
        }
    }
}
