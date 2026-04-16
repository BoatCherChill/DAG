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
    enum { Type = UserType + 15 }; // уникальный тип для qgraphicsitem_cast

    VisualNode(int id, NodeType type, QGraphicsItem* parent = nullptr); // конструктор узла (идентификатор, тип, родительский элемент)

    int getNodeId() const { return nodeId; }
    void setNodeId(int id) { nodeId = id; }
    NodeType getNodeType() const { return nodeType; }
    DataType getDataType() const { return dataType; }
    QString getInputData() const { return inputData; }
    bool isCalculated() const { return calculated; }
    const QVector<Arrow*>& getArrows() const { return arrows; }

    void setInputData(const QString& data);
    void setResult(double val);

    void addOutputData(const QString& data); // добавить строку результата
    void clearOutputData(); // очистить данные в узле
    void showOutputDialog(); // показать узел с результатами

    void addArrow(Arrow* arrow); // добавить стрелку
    void removeArrow(Arrow* arrow); // удалить стрелку
    void removeArrows(); // удалить все стрелки узла

    bool isCompatibleWith(VisualNode* other) const; // функция проверки совместимости типов узлов

    int type() const override { return Type; } // возврат типа для qgraphicsitem_cast

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override; // функция обработки двойного нажатия мыши
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override; // функция отрисовки узла
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override; // функция изменения позиции узла

private:
    QString normalizeDataString(const QString& data) const; // функция нормализации строки
    QList<double> extractNumbers(const QString& data) const; // функция извлечения чисел из строки
    bool validateNumberData(const QString& data); // функция проверки на отсутсвие чего либо кроме чисел

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
