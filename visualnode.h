#pragma once

#include <QGraphicsEllipseItem>
#include <QVector>
#include <QString>
#include <QList>

class Arrow;

enum class NodeType;
enum class DataType { NUMBER, TEXT };

class VisualNode : public QGraphicsEllipseItem {
public:
    enum { Type = UserType + 15 };

    VisualNode(int id, NodeType type, QGraphicsItem* parent = nullptr); // конструктор узла (идентификатор, тип, родительский элемент)

    // Геттеры
    int getNodeId() const { return nodeId; }
    void setNodeId(int id) { nodeId = id; }
    NodeType getNodeType() const { return nodeType; }
    DataType getDataType() const { return dataType; }
    QString getInputData() const { return inputData; }
    bool isCalculated() const { return calculated; }
    const QVector<Arrow*>& getArrows() const { return arrows; }

    // Сеттеры
    void setInputData(const QString& data);
    void setResult(double val);

    // Для OUTPUT узла
    void addOutputData(const QString& data);
    void clearOutputData();
    void showOutputDialog();

    // Управление связями
    void addArrow(Arrow* arrow);
    void removeArrow(Arrow* arrow);
    void removeArrows();

    // Проверка совместимости
    bool isCompatibleWith(VisualNode* other) const;

    int type() const override { return Type; }

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    QString normalizeDataString(const QString& data) const;
    QList<double> extractNumbers(const QString& data) const;
    bool validateNumberData(const QString& data);

    int nodeId;
    NodeType nodeType;
    DataType dataType;
    QString inputData;
    QString inputDataRaw;
    QList<double> numberArray;
    QStringList outputDataList;
    QVector<Arrow*> arrows;
    bool calculated;
    double value;
    bool outputDialogOpen;
};
