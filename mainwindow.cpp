
#include "mainwindow.h"
#include "diagramscene.h"
#include "visualnode.h"
#include "arrow.h"
#include <QtWidgets>
#include <QButtonGroup>
#include <QToolButton>

// конструктор окна приложения
MainWindow::MainWindow() {
    setupUI();
    createToolbar();
    setWindowTitle("Ациклические направленные графы");
    resize(1000, 700);
}

// деструктор окна приложения
MainWindow::~MainWindow() {
    QList<QWidget*> topWidgets = QApplication::topLevelWidgets();
    for (QWidget* widget : topWidgets) {
        if (widget != this && widget->isVisible()) {
            widget->close();
        }
    }
}

// функция настройки интерфейса
void MainWindow::setupUI() {
    QMenu* itemMenu = new QMenu(this);
    scene = new DiagramScene(itemMenu, this);
    scene->setSceneRect(-5000, -5000, 10000, 10000);
    scene->setBackgroundBrush(Qt::white);
    scene->setMode(DiagramScene::InsertNode);

    QObject::connect(scene, &DiagramScene::itemInserted, this, &MainWindow::addNode);
    QObject::connect(scene, &DiagramScene::arrowCreated, this, &MainWindow::onArrowCreated);
    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setDragMode(QGraphicsView::NoDrag);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(view, &QGraphicsView::customContextMenuRequested, this, &MainWindow::showContextMenu);
    setCentralWidget(view);
}

void MainWindow::createToolbar() {
    QToolBar* bar = addToolBar("");
    bar->setContextMenuPolicy(Qt::PreventContextMenu);

    QAction* deleteAction = bar->addAction("Удалить");
    QObject::connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteItem);

    bar->addSeparator();

    QButtonGroup* modeGroup = new QButtonGroup(this);
    modeGroup->setExclusive(true);

    node_button = new QToolButton;
    node_button->setText("Узлы");
    node_button->setCheckable(true);
    node_button->setChecked(true);

    arrow_button = new QToolButton;
    arrow_button->setText("Связи");
    arrow_button->setCheckable(true);

    modeGroup->addButton(node_button, 0);
    modeGroup->addButton(arrow_button, 1);

    bar->addWidget(node_button);
    bar->addWidget(arrow_button);

    QObject::connect(node_button, &QToolButton::clicked, [this]() {
        scene->setMode(DiagramScene::InsertNode);
        for (QGraphicsItem* item : scene->items()) {
            if (VisualNode* node = qgraphicsitem_cast<VisualNode*>(item)) {
                node->setFlag(QGraphicsItem::ItemIsMovable, true);
            }
        }
        view->setCursor(Qt::ArrowCursor);
    });

    QObject::connect(arrow_button, &QToolButton::clicked, [this]() {
        scene->setMode(DiagramScene::InsertArrow);
        for (QGraphicsItem* item : scene->items()) {
            if (VisualNode* node = qgraphicsitem_cast<VisualNode*>(item)) {
                node->setFlag(QGraphicsItem::ItemIsMovable, false);
            }
        }
        view->setCursor(Qt::CrossCursor);
    });

    bar->addSeparator();

    QPushButton* execBtn = new QPushButton("Выполнить");
    execBtn->setObjectName("executeButton");
    execBtn->setEnabled(false);
    bar->addWidget(execBtn);
    QObject::connect(execBtn, &QPushButton::clicked, this, &MainWindow::executeGraph);

    bar->addSeparator();
}

void MainWindow::showContextMenu(const QPoint& pos) {
    if (scene->getMode() == DiagramScene::InsertArrow) return;
    if (scene->itemAt(view->mapToScene(pos), QTransform())) return;

    menu_pos = view->mapToScene(pos);
    QMenu menu;
    QMenu* ioMenu = menu.addMenu("Ввод/Вывод");
    ioMenu->addAction("Ввод", [this]() { createNode(NodeType::INPUT); });
    ioMenu->addAction("Вывод", [this]() { createNode(NodeType::OUTPUT); });
    QMenu* numMenu = menu.addMenu("Для чисел");
    numMenu->addAction("Сумма", [this]() { createNode(NodeType::SUM); });
    numMenu->addAction("Медиана", [this]() { createNode(NodeType::MEDIAN); });
    numMenu->addAction("Среднее", [this]() { createNode(NodeType::AVERAGE); });
    QMenu* txtMenu = menu.addMenu("Для текста");
    txtMenu->addAction("Верхний регистр", [this]() { createNode(NodeType::TO_UPPER); });
    txtMenu->addAction("Нижний регистр", [this]() { createNode(NodeType::TO_LOWER); });
    menu.exec(view->viewport()->mapToGlobal(pos));
}

// функция создания узла на сцене
void MainWindow::createNode(NodeType type) {
    int id = graph.getNextId();
    VisualNode* node = new VisualNode(id, type);
    node->setPos(menu_pos);
    node->setFlag(QGraphicsItem::ItemIsMovable, true);
    scene->addItem(node);
    addNode(node);
    updateExecuteButton();
}

// функция добавления узла в логический граф
void MainWindow::addNode(VisualNode* node) {
    graph.addNode(node->pos().x(), node->pos().y(), node->getNodeType());
    Node* graphNode = graph.findNodeById(graph.getNextId() - 1);
    if (graphNode) {
        node->setNodeId(graphNode->nodeID);
        idToNode[graphNode->nodeID] = node;
        if (node->getNodeType() == NodeType::INPUT && !node->getInputData().isEmpty()) {
            graphNode->result = node->getInputData().toStdString();
            graphNode->calculated = true;
        }
    }
}

// функция удаления узла из логического графа
void MainWindow::removeNode(VisualNode* node) {
    if (!node->getNodeId())
            return;
    graph.deleteNode(node->getNodeId());
    idToNode.remove(node->getNodeId());
}

// функция добавления стрелки в логический граф
void MainWindow::addArrow(Arrow* arrow) {
    int fromId = arrow->startItem()->getNodeId();
    int toId = arrow->endItem()->getNodeId();
    graph.addRelation(fromId, toId);
    arrowToIds[arrow] = qMakePair(fromId, toId);
    arrow->setZValue(-1000);
}

// функция удаления стрелки из логического графа
void MainWindow::removeArrow(Arrow* arrow) {
    if (!arrowToIds.contains(arrow))
        return;
    auto conn = arrowToIds[arrow];
    graph.deleteRelation(conn.first, conn.second);
    arrowToIds.remove(arrow);
}

// функция проверки на циклы после создания стрелки
void MainWindow::onArrowCreated(Arrow* arrow) {
    arrow->setZValue(-1000);
    addArrow(arrow);
    if (!graph.isDAG()) {
        QMessageBox::warning(this, "Ошибка", "Эта связь создаст цикл в графе!");
        removeArrow(arrow);
        arrow->startItem()->removeArrow(arrow);
        arrow->endItem()->removeArrow(arrow);
        scene->removeItem(arrow);
        delete arrow;
    }
    updateExecuteButton();
}

// функция удаления объекта сцены
void MainWindow::deleteItem() {
    QList<QGraphicsItem*> selected = scene->selectedItems();
    for (QGraphicsItem* item : selected) {
        if (Arrow* arrow = qgraphicsitem_cast<Arrow*>(item)) {
            removeArrow(arrow);
            if (arrow->startItem())
                arrow->startItem()->removeArrow(arrow);
            if (arrow->endItem())
                arrow->endItem()->removeArrow(arrow);
            scene->removeItem(arrow);
            delete arrow;
        } else if (VisualNode* node = qgraphicsitem_cast<VisualNode*>(item)) {
            for (Arrow* arrow : node->getArrows()) {
                removeArrow(arrow);
                scene->removeItem(arrow);
                delete arrow;
            }
            removeNode(node);
            scene->removeItem(node);
            delete node;
        }
    }
    updateExecuteButton();
}

// функция запуска вычислений графа
void MainWindow::executeGraph() {
    for (QGraphicsItem* item : scene->items()) {
        if (VisualNode* node = qgraphicsitem_cast<VisualNode*>(item)) {
            if (!graph.findNodeById(node->getNodeId()))
                addNode(node);
        } else if (Arrow* arrow = qgraphicsitem_cast<Arrow*>(item)) {
            if (!arrowToIds.contains(arrow))
                addArrow(arrow);
        }
    }

    QMap<int, QString> savedInputData;
    for (auto it = idToNode.begin(); it != idToNode.end(); ++it) {
        VisualNode* visNode = it.value();
        if (visNode->getNodeType() == NodeType::INPUT && visNode->isCalculated()) {
            savedInputData[it.key()] = visNode->getInputData();
        }
    }

    for (auto it = idToNode.begin(); it != idToNode.end(); ++it) {
        Node* graphNode = graph.findNodeById(it.key());
        VisualNode* visNode = it.value();
        if (graphNode && visNode->getNodeType() == NodeType::INPUT && visNode->isCalculated()) {
            graphNode->result = visNode->getInputData().toStdString();
            graphNode->calculated = true;
        }
    }

    for (auto it = idToNode.begin(); it != idToNode.end(); ++it) {
        if (it.value()->getNodeType() == NodeType::OUTPUT) {
            it.value()->clearOutputData();
        }
    }

    graph.execute();

    for (auto it = savedInputData.begin(); it != savedInputData.end(); ++it) {
        Node* graphNode = graph.findNodeById(it.key());
        VisualNode* visNode = idToNode[it.key()];
        if (graphNode && visNode) {
            graphNode->result = it.value().toStdString();
            graphNode->calculated = true;
            visNode->setInputData(it.value());
        }
    }

    for (auto it = idToNode.begin(); it != idToNode.end(); ++it) {
        Node* graphNode = graph.findNodeById(it.key());
        VisualNode* visNode = it.value();
        if (graphNode && graphNode->calculated) {
            if (visNode->getNodeType() == NodeType::OUTPUT) {
                for (int prevId : graphNode->prev_nodes) {
                    Node* prev = graph.findNodeById(prevId);
                    if (prev && prev->calculated) {
                        visNode->addOutputData(QString::fromStdString(prev->result));
                    }
                }
            } else if (visNode->getNodeType() != NodeType::INPUT) {
                double val = QString::fromStdString(graphNode->result).toDouble();
                visNode->setResult(val);
            }
        }
    }

    scene->setMode(DiagramScene::InsertNode);

    if (node_button) {
        node_button->setChecked(true);
    }
    if (arrow_button) {
        arrow_button->setChecked(false);
    }

    for (QGraphicsItem* item : scene->items()) {
        if (VisualNode* node = qgraphicsitem_cast<VisualNode*>(item)) {
            node->setFlag(QGraphicsItem::ItemIsMovable, true);
        }
    }
    view->setCursor(Qt::ArrowCursor);

    QMessageBox::information(this, "Выполнение", "Граф успешно выполнен!");
}

//  функция обновления состояния кнопки "Выполнить"
void MainWindow::updateExecuteButton() {
    QPushButton* execBtn = findChild<QPushButton*>("executeButton");
    if (!execBtn) return;

    bool hasOutput = false;
    bool hasConnectionToOutput = false;

    for (QGraphicsItem* item : scene->items()) {
        if (VisualNode* node = qgraphicsitem_cast<VisualNode*>(item)) {
            if (node->getNodeType() == NodeType::OUTPUT) {
                hasOutput = true;
                if (!node->getArrows().isEmpty()) {
                    hasConnectionToOutput = true;
                }
            }
        }
    }

    execBtn->setEnabled(hasOutput && hasConnectionToOutput);
}
