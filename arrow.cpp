#include "arrow.h"
#include "visualnode.h"
#include <QPainter>
#include <QtMath>
#include <QGraphicsScene>

//конструктор стрелки
Arrow::Arrow(VisualNode* start, VisualNode* end) : current_start(start), current_end(end) {
    setFlag(QGraphicsItem::ItemIsSelectable, true); // подключить выделение стрелки
    setPen(QPen(Qt::black, 2));
    setZValue(-1000); // создавать стрелку под узлами
}

// функция, задающая ограничения для стрелки
QRectF Arrow::boundingRect() const {
    qreal extra = (pen().width() + 20) / 2.0; // сделать дополнительный отступ от узла
    return QRectF(line().p1(), QSizeF(line().p2().x() - line().p1().x(), line().p2().y() - line().p1().y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}

// функция рисования стрелки (инструмент для рисования, параметры объекта рисования, параметры для виджета)
void Arrow::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    // нарисовать линию
    QPen myPen = pen();
    myPen.setColor(Qt::black);

    painter->setPen(myPen);
    painter->setBrush(Qt::black);

    QPointF start_position = current_start->pos();
    QPointF end_position = current_end->pos();

    double delta_x = end_position.x() - start_position.x();
    double delta_y = end_position.y() - start_position.y();
    double length = sqrt(delta_x*delta_x + delta_y*delta_y);

    if (length < 0.1)
        return;

    double dirX = delta_x / length;
    double dirY = delta_y / length;
    double perpX = -dirY;
    double perpY = dirX;

    double offset = 35; // отступ от центра узла

    QPointF start_point(start_position.x() + dirX * offset, start_position.y() + dirY * offset);
    QPointF end_point(end_position.x() - dirX * offset, end_position.y() - dirY * offset);

    painter->drawLine(start_point, end_point);

    // Нарисовать наконечник
    qreal arrow_size = 15;
    QPointF arrow_left = end_point - QPointF(dirX * arrow_size - perpX * arrow_size/2, dirY * arrow_size - perpY * arrow_size/2);
    QPointF arrow_right = end_point - QPointF(dirX * arrow_size + perpX * arrow_size/2, dirY * arrow_size + perpY * arrow_size/2);

    arrow_head.clear();
    arrow_head << end_point << arrow_left << arrow_right;
    painter->drawPolygon(arrow_head);

    // Если стрелка выделена
    if (isSelected()) {
        painter->setPen(QPen(Qt::black, 1, Qt::DashLine));
            painter->setBrush(Qt::NoBrush);

            QLineF lineUp(start_point, end_point);
            QLineF lineDown(start_point, end_point);
            lineUp.translate(0, 6);
            lineDown.translate(0, -6);
            painter->drawLine(lineUp);
            painter->drawLine(lineDown);
        }
}

// функция обновления координат стрелки после обновления координат начального или конечного узла
void Arrow::updatePosition() {
    if (!current_start || !current_end) return;
    QLineF line(mapFromItem(current_start, 0, 0), mapFromItem(current_end, 0, 0)); // пересчитать положение в локальных координатах (относительно стрелки)
    setLine(line);
}
