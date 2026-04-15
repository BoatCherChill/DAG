#ifndef EDGE_H
#define EDGE_H

#include <QGraphicsLineItem>

class Node;

class Edge : public QGraphicsLineItem {
public:
    enum { Type = UserType + 4 };

    Edge(Node* from, Node* to, QGraphicsItem* parent = nullptr);

    int type() const override { return Type; }
    QRectF boundingRect() const override;
    void updatePosition();

    Node* getFrom() const { return fromNode; }
    Node* getTo() const { return toNode; }

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    Node* fromNode;
    Node* toNode;
    QPolygonF arrowHead;
};

#endif
