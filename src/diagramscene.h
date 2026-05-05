#pragma once
#include <QGraphicsScene>
#include <QMenu>

class VisualNode;
class Arrow;

class DiagramScene : public QGraphicsScene {
    Q_OBJECT
public:
    //режимы вставки узлов и стрелок
    enum Mode {
        InsertNode,
        InsertArrow
    };

    DiagramScene(QObject* parent = nullptr); // конструктор сцены (контекстное меню, родительский объект)

    void setMode(Mode mode); // функция установки режима (режим)
    Mode getMode() const { return my_mode; }

signals:
    void arrowCreated(Arrow* arrow); // сигнал о создании стрелки

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override; // функция обработки нажатия левой кнопки мыши
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override; //функция обновления временной стрелки при движении мыши
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override; // функция создания стрелки на экране после отпускания мыши

private:
    Mode my_mode;  // текущий режим
    QGraphicsLineItem* line = nullptr; // временная линия рисования связи
};

