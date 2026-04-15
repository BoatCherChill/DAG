#ifndef DIAGRAMITEM_H
#define DIAGRAMITEM_H

#include <QGraphicsPolygonItem>
#include <QVector>

QT_BEGIN_NAMESPACE
class QPixmap;
class QGraphicsSceneContextMenuEvent;
class QMenu;
class QPolygonF;
QT_END_NAMESPACE

class Arrow;

enum class DataType {
    NUMBER,   // Числовые данные
    TEXT      // Текстовые данные
};

class DiagramItem : public QGraphicsPolygonItem
{
public:
    enum { Type = UserType + 15 };

    enum DiagramType {
        Sum,
        Median,
        Average,
        Text_Concat,
        To_Lower,
        To_Upper,
        Input,
        Output
    };

    DiagramItem(DiagramType diagramType, QMenu *contextMenu, QGraphicsItem *parent = nullptr);

    void removeArrow(Arrow *arrow);
    void removeArrows();
    DiagramType diagramType() const { return myDiagramType; }
    QPolygonF polygon() const { return myPolygon; }
    void addArrow(Arrow *arrow);
    QPixmap image() const;
    int type() const override { return Type; }

    void setValue(double value);
    double getValue() const { return myValue; }
    void setInputData(const QString &data);
    QString getInputData() const { return myInputData; }
    bool isCalculated() const { return calculated; }
    void setCalculated(bool calc) { calculated = calc; }
    void setNodeId(int id) { nodeId = id; }
    int getNodeId() const { return nodeId; }

    DataType getDataType() const { return myDataType; }
    void setDataType(DataType type) { myDataType = type; update(); }
    bool isCompatibleWith(DiagramItem *other) const;

    QString getTextData() const { return myTextData; }
    void setTextData(const QString &text);
    QList<double> getInputArray() const { return myInputArray; }
    void setInputArray(const QList<double>& array);

    // Методы для Output узла
    void addOutputData(const QString &data);
    void clearOutputData();
    void showOutputDialog();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    int nodeId;
    DiagramType myDiagramType;
    QPolygonF myPolygon;
    QMenu *myContextMenu;
    QVector<Arrow *> arrows;

    double myValue;
    QString myInputData;
    bool calculated;

    QList<double> myInputArray;
    DataType myDataType;
    QString myTextData;

    // Для Output узла
    QStringList outputDataList;
    bool outputDialogOpen;
};

#endif
