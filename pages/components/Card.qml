// Fundo de cartão do Windows 11: levemente mais claro, borda fina, cantos de 5 px
import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    property bool hovered: false
    property bool pressed: false
    radius: 5
    color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b,
                   pressed ? 0.035 : (hovered ? 0.075 : 0.05))
    Behavior on color { ColorAnimation { duration: 120 } }
    border.width: 1
    border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.06)
}
