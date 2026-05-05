#include "diagramscene.h"
#include "visualnode.h"
#include "arrow.h"
#include "graph.h"
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>

//конструктор сцены
DiagramScene::DiagramScene(QObject* parent)
    : QGraphicsScene(parent) {}


// функция установки режима работы сцены
void DiagramScene::setMode(Mode mode) {
    my_mode = mode;
}

// функция обработки нажатия левой кнопки мыши
void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    // Режим рисования связей
    if (my_mode == InsertArrow) {
        line = new QGraphicsLineItem(QLineF(event->scenePos(), event->scenePos()));
        line->setPen(QPen(Qt::black, 2));
        addItem(line);
    }

    QGraphicsScene::mousePressEvent(event);
}

//функция обновления временной стрелки при движении мыши
void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    // Обновление временной линии при движении мыши
    if (my_mode == InsertArrow && line) {
        line->setLine(QLineF(line->line().p1(), event->scenePos()));
    }
    QGraphicsScene::mouseMoveEvent(event);
}

// функция создания стрелки на экране после отпускания мыши
void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (line && my_mode == InsertArrow) {
        // найти узлы в начальной и конечной точках
        QList<QGraphicsItem*> start_items = items(line->line().p1());
        QList<QGraphicsItem*> end_items = items(line->line().p2());

        // убрать саму линию из списков
        if (start_items.contains(line))
            start_items.removeOne(line);
        if (end_items.contains(line))
            end_items.removeOne(line);

        // удалить временную линию
        removeItem(line);
        delete line;
        line = nullptr;

        // проверить, можно ли создать стрелку
        if (!start_items.isEmpty() && !end_items.isEmpty() && start_items.first() != end_items.first()) {
            VisualNode* start = qgraphicsitem_cast<VisualNode*>(start_items.first());
            VisualNode* end = qgraphicsitem_cast<VisualNode*>(end_items.first());

            // Запрет на стрелки в узел Input
            if (end && end->getNodeType() == NodeType::INPUT) {
                QMessageBox::warning(nullptr, "Ошибка", "Нельзя создавать стрелки в узел Input!");
                return;
            }

            // Запрет стрелки из узла Output
            if (start && start->getNodeType() == NodeType::OUTPUT) {
                QMessageBox::warning(nullptr, "Ошибка", "Нельзя создавать стрелки из узла Output!");
                return;
            }

            // Запрет нескольких стрелок к узлу Output
            if (end && end->getNodeType() == NodeType::OUTPUT && !end->getArrows().isEmpty()) {
                QMessageBox::warning(nullptr, "Ошибка", "У узла Output может быть только одна стрелка!");
                return;
            }

            // Проверка совместимости типов
            if (start && end && start->isCompatibleWith(end)) {
                Arrow* arrow = new Arrow(start, end);
                arrow->setZValue(-1000);
                start->addArrow(arrow);
                end->addArrow(arrow);
                addItem(arrow);
                arrow->updatePosition();
                emit arrowCreated(arrow);
            } else if (start && end) {
                QMessageBox::warning(nullptr, "Ошибка", "Нельзя соединить узлы, которые работают с разными типами данных!");
            }
        }
    }
    QGraphicsScene::mouseReleaseEvent(event);
}
