// Linha do mixer: ícone e nome do aplicativo, valor e slider próprio
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: row
    property string title
    property string iconName
    property var stream: null
    property int maxVolume: 65536

    Layout.fillWidth: true
    implicitHeight: 54

    Rectangle { width: parent.width; height: 1; color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.07) }
    Kirigami.Icon {
        x: 56
        anchors.verticalCenter: parent.verticalCenter
        width: 24
        height: 24
        source: row.iconName
    }
    QQC2.Label {
        x: 92
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - x - 260
        text: row.title
        elide: Text.ElideRight
    }
    RowLayout {
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10
        QQC2.Label {
            Layout.preferredWidth: 30
            horizontalAlignment: Text.AlignRight
            text: row.stream ? Math.round((row.stream.muted ? 0 : row.stream.volume) / row.maxVolume * 100) : 0
        }
        QQC2.Slider {
            Layout.preferredWidth: 170
            from: 0
            to: row.maxVolume
            value: row.stream ? (row.stream.muted ? 0 : row.stream.volume) : 0
            onMoved: {
                row.stream.volume = Math.round(value / row.maxVolume * 100) * row.maxVolume / 100;
                row.stream.muted = value < row.maxVolume / 200;
            }
        }
    }
}
