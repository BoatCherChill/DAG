#include "diagramscene.h"
#include "visualnode.h"
#include "arrow.h"
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>

// ==================== КОНСТРУКТОР ====================

DiagramScene::DiagramScene(QMenu* itemMenu, QObject* parent)
    : QGraphicsScene(parent), my_menu(itemMenu) {}

// ==================== НАСТРОЙКА РЕЖИМОВ ====================

void DiagramScene::setMode(Mode mode) {
    my_mode = mode;
}

void DiagramScene::setItemType(NodeType type) {
    my_node_type = type;
}

// ==================== ПОИСК УЗЛА В ТОЧКЕ ====================

VisualNode* DiagramScene::findNode(const QPointF& pos) {
    for (QGraphicsItem* item : items(pos)) {
        if (VisualNode* node = qgraphicsitem_cast<VisualNode*>(item)) {
            return node;
        }
    }
    return nullptr;
}

// ==================== ОБРАБОТКА СОБЫТИЙ МЫШИ ====================

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    // Режим рисования связей
    if (my_mode == InsertLine) {
        line = new QGraphicsLineItem(QLineF(event->scenePos(), event->scenePos()));
        line->setPen(QPen(myLineColor, 2));
        addItem(line);
    }

    QGraphicsScene::mousePressEvent(event);
}

void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    // Обновление временной линии при движении мыши
    if (my_mode == InsertLine && line) {
        line->setLine(QLineF(line->line().p1(), event->scenePos()));
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    // Завершение рисования связи
    if (line && my_mode == InsertLine) {
        // Находим узлы в начальной и конечной точках
        QList<QGraphicsItem*> startItems = items(line->line().p1());
        QList<QGraphicsItem*> endItems = items(line->line().p2());

        // Убираем саму линию из списков
        if (startItems.contains(line))
            startItems.removeOne(line);
        if (endItems.contains(line))
            endItems.removeOne(line);

        // Удаляем временную линию
        removeItem(line);
        delete line;
        line = nullptr;

        // Проверяем, можно ли создать стрелку
        if (!startItems.isEmpty() && !endItems.isEmpty() && startItems.first() != endItems.first()) {
            VisualNode* start = qgraphicsitem_cast<VisualNode*>(startItems.first());
            VisualNode* end = qgraphicsitem_cast<VisualNode*>(endItems.first());

            if (end && end->getNodeType() == NodeType::INPUT) {
                QMessageBox::warning(nullptr, "Ошибка", "Нельзя создавать связи в узел Input!");
                return;
            }

            // Запрет связи из узла Output
            if (start && start->getNodeType() == NodeType::OUTPUT) {
                QMessageBox::warning(nullptr, "Ошибка", "Нельзя создавать связи из узла Output!");
                return;
            }

            // У Output узла может быть только одна связь
            if (end && end->getNodeType() == NodeType::OUTPUT && !end->getArrows().isEmpty()) {
                QMessageBox::warning(nullptr, "Ошибка", "У узла Output может быть только одна связь!");
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
                QMessageBox::warning(nullptr, "Ошибка", "Нельзя соединить эти узлы!");
            }
        }
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

