#pragma once
#include <QGraphicsScene>
#include <QMenu>
#include "graph.h"

class VisualNode;
class Arrow;

class DiagramScene : public QGraphicsScene {
    Q_OBJECT
public:
    enum Mode { InsertNode, InsertLine }; //режимы вставки узлов и стрелок

    DiagramScene(QMenu* itemMenu, QObject* parent = nullptr); // конструктор графа

    void setMode(Mode mode); // функция изменения режима
    Mode getMode() const { return my_mode; }

signals:
    void itemInserted(VisualNode* item);
    void arrowCreated(Arrow* arrow);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override; // функция, обрабатывающая нажатия на мышь
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override; // функция рисования стрелки мышью
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override; // функция создания стрелки на экране

private:
    VisualNode* findNode(const QPointF& pos); // функция поиска узла в графе

    Mode my_mode = InsertNode;  // по умолчанию режим работы с узлами
    NodeType my_node_type = NodeType::INPUT;
    QMenu* my_menu;
    QGraphicsLineItem* line = nullptr;
};

