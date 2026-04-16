#pragma once
#include <QGraphicsScene>
#include <QMenu>
#include "graph.h"

class VisualNode;
class Arrow;

class DiagramScene : public QGraphicsScene {
    Q_OBJECT
public:
    enum Mode { InsertNode, InsertArrow }; //режимы вставки узлов и стрелок

    DiagramScene(QMenu* itemMenu, QObject* parent = nullptr); // конструктор сцены (контекстное меню, родительский объект)

    void setMode(Mode mode); // функция установки режима (режим)
    Mode getMode() const { return my_mode; }

signals:
    void itemInserted(VisualNode* item); // согнал о создании узла (объект
    void arrowCreated(Arrow* arrow); // сигнал о создании стрелки

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override; // функция обработки нажатия левой кнопки мыши
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override; //функция обновления временной стрелки при движении мыши
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override; // функция создания стрелки на экране после отпускания мыши

private:
    VisualNode* findNode(const QPointF& pos); // функция поиска узла в графе

    Mode my_mode;  // текущий режим
    QMenu* my_menu; // контекстное мегю для создания узлов
    QGraphicsLineItem* line = nullptr; // временная линия рисования связи
};

