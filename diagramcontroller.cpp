#include "diagramcontroller.h"
#include "arrow.h"
#include "dag.h"
#include "operations.h"
#include <QMessageBox>

// Конструктор контроллера
DiagramController::DiagramController(DiagramScene *scene, QObject *parent)
    : QObject(parent), scene(scene)
{
}


NodeType DiagramController::getNodeTypeFromDiagramType(DiagramItem::DiagramType diagramType)
{
    switch (diagramType) {
        case DiagramItem::Input: return NodeType::INPUT;
        case DiagramItem::Output: return NodeType::OUTPUT;
        case DiagramItem::Sum: return NodeType::SUM;
        case DiagramItem::Median: return NodeType::MEDIAN;
        case DiagramItem::Average: return NodeType::AVERAGE;
        case DiagramItem::Text_Concat: return NodeType::TEXT_CONCAT;
        case DiagramItem::To_Upper: return NodeType::TO_UPPER;
        case DiagramItem::To_Lower: return NodeType::TO_LOWER;
        default: return NodeType::INPUT;
    }
}


DiagramItem::DiagramType DiagramController::getDiagramTypeFromNodeType(NodeType nodeType)
{
    switch (nodeType) {
        case NodeType::INPUT: return DiagramItem::Input;
        case NodeType::OUTPUT: return DiagramItem::Output;
        case NodeType::SUM: return DiagramItem::Sum;
        case NodeType::MEDIAN: return DiagramItem::Median;
        case NodeType::AVERAGE: return DiagramItem::Average;
        case NodeType::TEXT_CONCAT: return DiagramItem::Text_Concat;
        case NodeType::TO_UPPER: return DiagramItem::To_Upper;
        case NodeType::TO_LOWER: return DiagramItem::To_Lower;
        default: return DiagramItem::Input;
    }
}

// добавление узла в граф
void DiagramController::addItemToGraph(DiagramItem *item)
{
    if (!item || itemToId.contains(item)) return;  // Уже добавлен

    int x = item->pos().x();
    int y = item->pos().y();
    NodeType type = getNodeTypeFromDiagramType(item->diagramType());

    graph.add_node(x, y, type);  // Добавляем узел в граф

    Node* node = graph.find_node_by_id(graph.get_next_id() - 1);
    if (node) {
        item->setNodeId(node->nodeID);           // Сохраняем ID в визуальном узле
        idToItem[node->nodeID] = item;           // ID -> визуальный элемент
        itemToId[item] = node->nodeID;           // Визуальный элемент -> ID

        // Если это Input узел и у него уже есть данные, сохраняем их
        if (item->diagramType() == DiagramItem::Input && item->isCalculated()) {
            node->result = item->getInputData().toStdString();
            node->calculated = true;
        }
    }
}

// удаление узла из графа
void DiagramController::removeItemFromGraph(DiagramItem *item)
{
    if (!itemToId.contains(item)) return;

    int id = itemToId[item];
    graph.delete_node(id);  // Удаляем из графа

    idToItem.remove(id);
    itemToId.remove(item);
}

// добавление связи в граф
void DiagramController::addConnection(Arrow *arrow)
{
    DiagramItem *startItem = arrow->startItem();
    DiagramItem *endItem = arrow->endItem();

    if (!startItem || !endItem) return;
    if (!itemToId.contains(startItem) || !itemToId.contains(endItem)) return;

    int fromId = itemToId[startItem];
    int toId = itemToId[endItem];

    graph.addEdge(fromId, toId);  // Добавляем ребро в граф
    arrowConnections[arrow] = qMakePair(fromId, toId);  // Сохраняем для возможного удаления
}

// удаление связи из графа
void DiagramController::removeConnection(Arrow *arrow)
{
    if (!arrowConnections.contains(arrow)) return;

    auto conn = arrowConnections[arrow];
    graph.del_relation_by_nodes(conn.first, conn.second);  // Удаляем ребро
    arrowConnections.remove(arrow);
}

// выполненеи графа
void DiagramController::executeGraph()
{
    syncFromScene();

    vector<Node>& nodes = graph.get_nodes();

    // Проверка на пустой граф
    if (nodes.empty()) {
        QMessageBox::information(nullptr, "Информация",
            "Граф пуст. Добавьте узлы для выполнения.");
        return;
    }

    // Проверка на наличие узлов ввода
    bool hasInput = false;
    for (const auto& node : nodes) {
        if (node.type == NodeType::INPUT) {
            hasInput = true;
            break;
        }
    }

    if (!hasInput) {
        QMessageBox::warning(nullptr, "Ошибка",
            "В графе нет узлов ввода! Добавьте хотя бы один узел Input.");
        return;
    }

    // Проверка на циклы
    if (!is_DAG(nodes)) {
        QMessageBox::warning(nullptr, "Ошибка",
            "Граф содержит циклы! Выполнение невозможно.\n"
            "Удалите связи, создающие циклы.");
        return;
    }

    // Очищаем все узлы вывода перед выполнением
    for (auto it = idToItem.begin(); it != idToItem.end(); ++it) {
        DiagramItem *item = it.value();
        if (item->diagramType() == DiagramItem::Output) {
            item->clearOutputData();
        }
    }

    // Топологическая сортировка
    vector<int> order = top_sort(nodes);

    if (order.empty() && !nodes.empty()) {
        QMessageBox::warning(nullptr, "Ошибка",
            "Не удалось выполнить топологическую сортировку.");
        return;
    }

    // Выполняем узлы
    bool hasErrors = false;

    for (int nodeId : order) {
        Node* node = graph.find_node_by_id(nodeId);
        if (!node) continue;

        if (node->type == NodeType::OUTPUT) {
            for (int prevId : node->prevNodes) {
                Node* prev = graph.find_node_by_id(prevId);
                if (prev && prev->calculated) {
                    QString resultStr = QString::fromStdString(prev->result);
                    DiagramItem *outputItem = idToItem[nodeId];
                    if (outputItem) {
                        outputItem->addOutputData(resultStr);
                    }
                    if (node->result.empty()) {
                        node->result = prev->result;
                    } else {
                        node->result += "\n" + prev->result;
                    }
                }
            }
            node->calculated = true;
            continue;
        }

        string allData;
        for (int prevId : node->prevNodes) {
            Node* prev = graph.find_node_by_id(prevId);
            if (prev && prev->calculated) {
                if (!allData.empty()) allData += " ";
                allData += prev->result;
            }
        }

        // Для узлов, у которых нет входов (кроме INPUT)
        if (node->type != NodeType::INPUT && node->prevNodes.empty()) {
            // Пропускаем узлы без входов
            continue;
        }

        try {
            switch (node->type) {
                case NodeType::INPUT:
                    node->calculated = true;
                    break;

                case NodeType::SUM:
                    if (allData.empty()) {
                        node->result = "0";
                    } else {
                        node->result = sum(allData);
                    }
                    node->calculated = true;
                    break;

                case NodeType::MEDIAN:
                    if (allData.empty()) {
                        node->result = "0";
                    } else {
                        node->result = median(allData);
                    }
                    node->calculated = true;
                    break;

                case NodeType::AVERAGE:
                    if (allData.empty()) {
                        node->result = "0";
                    } else {
                        node->result = average(allData);
                    }
                    node->calculated = true;
                    break;

                case NodeType::TEXT_CONCAT:
                    node->result = concat(allData);
                    node->calculated = true;
                    break;

                case NodeType::TO_UPPER:
                    if (allData.empty()) {
                        node->result = "";
                    } else {
                        node->result = toUpper(allData);
                    }
                    node->calculated = true;
                    break;

                case NodeType::TO_LOWER:
                    if (allData.empty()) {
                        node->result = "";
                    } else {
                        node->result = toLower(allData);
                    }
                    node->calculated = true;
                    break;

                default:
                    break;
            }
        } catch (...) {
            hasErrors = true;
            QMessageBox::warning(nullptr, "Ошибка",
                QString("Ошибка при выполнении узла %1").arg(nodeId));
        }
    }

    syncToScene();

    if (hasErrors) {
        QMessageBox::warning(nullptr, "Выполнение завершено с ошибками",
            "Граф выполнен, но были ошибки в некоторых узлах.");
    } else {
        QMessageBox::information(nullptr, "Выполнение завершено",
            "Граф успешно выполнен!\n"
            "Дважды кликните на узле Output для просмотра результатов.");
    }
}
// загрузка данных в узлы графа
void DiagramController::syncFromScene()
{
    for (auto it = idToItem.begin(); it != idToItem.end(); ++it) {
        DiagramItem *item = it.value();
        Node* node = graph.find_node_by_id(it.key());
        if (node && item->diagramType() == DiagramItem::Input && item->isCalculated()) {
            node->result = item->getInputData().toStdString();
            node->calculated = true;
        }
    }
}


// загрузка результатов в узлы на сцену
void DiagramController::syncToScene()
{
    for (auto it = idToItem.begin(); it != idToItem.end(); ++it) {
        Node* node = graph.find_node_by_id(it.key());
        if (node) {
            DiagramItem *item = it.value();
            if (node->calculated) {
                // Преобразуем строковый результат в число и отображаем на узле
                double value = QString::fromStdString(node->result).toDouble();
                item->setValue(value);
                item->setInputData(QString::fromStdString(node->result));
            }
        }
    }
}

bool DiagramController::isGraphAcyclic() const
{
    const vector<Node>& nodes = graph.get_nodes();
    return is_DAG(nodes);
}
