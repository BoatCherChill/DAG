#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QToolButton>
#include <QMap>
#include "graph.h"

class DiagramScene;
class VisualNode;
class Arrow;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

private slots:
    void showContextMenu(const QPoint& pos);
    void createNode(NodeType type);
    void deleteItem();
    void executeGraph();
    void onArrowCreated(Arrow* arrow);

private:
    void setupUI();
    void createToolbar();
    void addNode(VisualNode* node);
    void removeNode(VisualNode* node);
    void addConnection(Arrow* arrow);
    void removeConnection(Arrow* arrow);
    void updateExecuteButton();
    bool isGraphAcyclic();
    void showInstruction();

    DiagramScene* scene;
    QGraphicsView* view;
    QPointF menu_pos;
    Graph graph;
    QMap<int, VisualNode*> idToNode;
    QMap<Arrow*, QPair<int, int>> arrowToIds;

    QToolButton* node_button;
    QToolButton* arrow_button;
};
