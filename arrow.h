#pragma once

#include <QGraphicsLineItem>
#include <QPolygonF>

class VisualNode;

class Arrow : public QGraphicsLineItem {
public:
    Arrow(VisualNode* start, VisualNode* end); // конструктор стрелки
    void updatePosition(); // функция обновления координат стрелки после изменения координат узлов
    VisualNode* startItem() const { return current_start; }
    VisualNode* endItem() const { return current_end; }

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override; // функция рисования стрелки (инструмент для рисования, параметры объекта рисования, параметры для виджета)
    QRectF boundingRect() const override; // функция, задающая ограничения для стрелки

private:
    VisualNode* current_start;
    VisualNode* current_end;
    QPolygonF arrow_head;
};
