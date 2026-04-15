#ifndef DIAGRAMCONTROLLER_H
#define DIAGRAMCONTROLLER_H

#include "graph.h"
#include "diagramitem.h"
#include "diagramscene.h"
#include <QObject>
#include <QMap>
#include <QPair>

class Arrow;
class DiagramItem;
class DiagramScene;


class DiagramController : public QObject
{
    Q_OBJECT

public:
    explicit DiagramController(DiagramScene *scene, QObject *parent = nullptr);

    // Управление узлами
    void addItemToGraph(DiagramItem *item);      // Добавить узел в граф
    void removeItemFromGraph(DiagramItem *item); // Удалить узел из графа

    // Управление связями
    void addConnection(Arrow *arrow);            // Добавить связь
    void removeConnection(Arrow *arrow);         // Удалить связь

    // Выполнение графа
    void executeGraph();                         // Запуск вычислений

    // Синхронизация между сценой и графом
    void syncFromScene();    // Загрузить данные из сцены в граф
    void syncToScene();      // Выгрузить результаты из графа на сцену

    const Graph& getGraph() const { return graph; }
    Graph& getGraph() { return graph; }

    bool isGraphAcyclic() const;

private:
    DiagramScene *scene;                                    // Указатель на сцену
    Graph graph;                                            // Логический граф
    QMap<int, DiagramItem*> idToItem;                      // ID узла -> визуальный элемент
    QMap<DiagramItem*, int> itemToId;                      // Визуальный элемент -> ID узла
    QMap<Arrow*, QPair<int, int>> arrowConnections;        // Стрелка -> (from, to)

    // Конвертация типов между визуальным и логическим представлением
    NodeType getNodeTypeFromDiagramType(DiagramItem::DiagramType diagramType);
    DiagramItem::DiagramType getDiagramTypeFromNodeType(NodeType nodeType);
};

#endif
