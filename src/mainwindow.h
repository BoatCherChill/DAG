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
    MainWindow(); // конструктор главного кона
    ~MainWindow(); // деструктор главного окна

private slots:
    void showContextMenu(const QPoint& pos); // контекстное меню
    void createNode(NodeType type); // функция создания узла на сцене
    void deleteItem(); // функция удаления объекта
    void executeGraph(); // функция запуска вычисления графа
    void onArrowCreated(Arrow* arrow); // функция проверки на циклы после создания стрелки

private:
    void setupUI(); // функция настройки пользовательсткого интерфейса
    void createToolbar(); // функция создания панели инструментов
    void addNode(VisualNode* node); // функция добавления узла в логический граф
    void removeNode(VisualNode* node); // функция улаления узла со сцены
    void addArrow(Arrow* arrow); // функция добавления стрелки на сцену
    void removeArrow(Arrow* arrow); // функция удаления стрелки со сцены
    void updateExecuteButton(); // функция обновления состояния кнопки "Выполнить"

    DiagramScene* scene;  // графическая сцена
    QGraphicsView* view; // виджет для отоюражения сцены
    QPointF menu_pos; // позиция создания узла
    Graph graph; // граф
    QMap<int, VisualNode*> idToNode; //словарь ID-узел
    QMap<Arrow*, QPair<int, int>> arrowToIds; //словарь стрелка-(координаты концов)

    QToolButton* node_button; // кнопка "Узлы"
    QToolButton* arrow_button; // кнопка "Связи"
};
