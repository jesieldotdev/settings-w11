// Seta fina do Windows: › (fechado), ⌄/⌃ (aberto), ↗ (abre fora)
import QtQuick
import org.kde.kirigami as Kirigami

Canvas {
    id: c
    property string kind: "right" // right | down | up | external
    width: 12
    height: 12
    onKindChanged: requestPaint()
    onPaint: {
        const g = getContext("2d");
        g.reset();
        g.strokeStyle = Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.75);
        g.lineWidth = 1.3;
        g.lineCap = "round";
        g.lineJoin = "round";
        g.beginPath();
        if (kind === "right") { g.moveTo(4, 1); g.lineTo(9, 6); g.lineTo(4, 11); }
        else if (kind === "down") { g.moveTo(1, 4); g.lineTo(6, 9); g.lineTo(11, 4); }
        else if (kind === "up") { g.moveTo(1, 8); g.lineTo(6, 3); g.lineTo(11, 8); }
        else { g.moveTo(2, 10); g.lineTo(10, 2); g.moveTo(5, 2); g.lineTo(10, 2); g.lineTo(10, 7); }
        g.stroke();
    }
}
