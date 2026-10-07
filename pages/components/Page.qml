// Página rolável das Configurações: coluna de cartões com o fundo da janela
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Rectangle {
    id: root
    default property alias content: column.data
    color: "transparent" // o fundo é o da janela (QQuickWidget)

    QQC2.ScrollView {
        id: scroll
        anchors.fill: parent
        contentWidth: availableWidth
        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        ColumnLayout {
            id: column
            width: scroll.availableWidth - 8
            spacing: 3
        }
    }
}
