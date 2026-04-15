#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include <QGraphicsScene>
#include <QMenu>
#include "graph.h"

class VisualNode;
class Arrow;

class DiagramScene : public QGraphicsScene {
    Q_OBJECT
public:
    enum Mode { MoveItem, InsertItem, InsertLine };

    DiagramScene(QMenu* itemMenu, QObject* parent = nullptr);

    void setMode(Mode mode);
    void setItemType(NodeType type);
    Mode getMode() const { return myMode; }

signals:
    void itemInserted(VisualNode* item);
    void arrowCreated(Arrow* arrow);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    VisualNode* findNodeAt(const QPointF& pos);

    Mode myMode = MoveItem;
    NodeType myItemType = NodeType::INPUT;
    QMenu* myItemMenu;
    QGraphicsLineItem* line = nullptr;
    QColor myItemColor = Qt::white;
    QColor myLineColor = Qt::black;
};

#endif
