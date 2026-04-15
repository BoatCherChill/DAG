#include "mainwindow.h"
#include "diagramscene.h"
#include "visualnode.h"
#include "arrow.h"
#include <QtWidgets>
#include <QButtonGroup>
#include <QToolButton>

// ==================== КОНСТРУКТОРЫ И ДЕСТРУКТОРЫ ====================

MainWindow::MainWindow() {
    setupUI();           // Настройка пользовательского интерфейса
    createToolbar();     // Создание панели инструментов
    setWindowTitle("Ациклические направленные графы");  // Установка заголовка окна
    resize(1000, 700);   // Установка начального размера окна
}

MainWindow::~MainWindow() {
    // Получаем список всех верхнеуровневых виджетов и закрываем их
    QList<QWidget*> topWidgets = QApplication::topLevelWidgets();
    for (QWidget* widget : topWidgets) {
        if (widget != this && widget->isVisible()) {
            widget->close();
        }
    }
}

// ==================== НАСТРОЙКА ИНТЕРФЕЙСА ====================

void MainWindow::setupUI() {
    QMenu* itemMenu = new QMenu(this);                     // Создание контекстного меню для узлов
    scene = new DiagramScene(itemMenu, this);              // Создание графической сцены
    scene->setSceneRect(-5000, -5000, 10000, 10000);       // Установка границ сцены (большое поле)
    scene->setBackgroundBrush(Qt::white);                  // Установка белого фона сцены
    scene->setMode(DiagramScene::InsertNode);                // Установка начального режима - перемещение

    QObject::connect(scene, &DiagramScene::itemInserted, this, &MainWindow::addNode);
    QObject::connect(scene, &DiagramScene::arrowCreated, this, &MainWindow::onArrowCreated);
    view = new QGraphicsView(scene, this);                 // Создание графического представления
    view->setRenderHint(QPainter::Antialiasing);           // Включение сглаживания
    view->setDragMode(QGraphicsView::NoDrag);
    view->setContextMenuPolicy(Qt::CustomContextMenu);     // Включение пользовательского контекстного меню
    QObject::connect(view, &QGraphicsView::customContextMenuRequested, this, &MainWindow::showContextMenu);
    setCentralWidget(view);                                // Установка представления как центрального виджета
}

void MainWindow::createToolbar() {
    QToolBar* bar = addToolBar("");                        // Создание панели инструментов без названия
    bar->setContextMenuPolicy(Qt::PreventContextMenu);     // Запрет на скрытие панели через контекстное меню

    QAction* deleteAction = bar->addAction("Удалить");     // Создание кнопки "Удалить"
    QObject::connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteItem); // Подключение удаления

    bar->addSeparator();                                   // Разделитель

    QButtonGroup* modeGroup = new QButtonGroup(this);      // Группа для кнопок режимов
    modeGroup->setExclusive(true);                         // Только одна кнопка может быть нажата

    node_button = new QToolButton;                           // Кнопка "Выбрать"
    node_button->setText("Узлы");
    node_button->setCheckable(true);                         // С возможностью нажатия
    node_button->setChecked(true);                           // Нажата по умолчанию

    arrow_button = new QToolButton;                            // Кнопка "Связи"
    arrow_button->setText("Связи");
    arrow_button->setCheckable(true);                          // С возможностью нажатия

    modeGroup->addButton(node_button, 0);                    // Добавление в группу
    modeGroup->addButton(arrow_button, 1);                     // Добавление в группу

    bar->addWidget(node_button);                             // Добавление на панель
    bar->addWidget(arrow_button);                              // Добавление на панель

    // Обработчик для кнопки "Выбрать" - режим перемещения узлов
    QObject::connect(node_button, &QToolButton::clicked, [this]() {
        scene->setMode(DiagramScene::InsertNode);            // Установка режима перемещения
        for (QGraphicsItem* item : scene->items()) {       // Перебор всех элементов сцены
            if (VisualNode* node = qgraphicsitem_cast<VisualNode*>(item)) {
                node->setFlag(QGraphicsItem::ItemIsMovable, true); // Включение перемещения узлов
            }
        }
        view->setCursor(Qt::ArrowCursor);                  // Установка курсора-стрелки
    });

    // Обработчик для кнопки "Связи" - режим создания связей
    QObject::connect(arrow_button, &QToolButton::clicked, [this]() {
        scene->setMode(DiagramScene::InsertLine);          // Установка режима рисования связей
        for (QGraphicsItem* item : scene->items()) {       // Перебор всех элементов сцены
            if (VisualNode* node = qgraphicsitem_cast<VisualNode*>(item)) {
                node->setFlag(QGraphicsItem::ItemIsMovable, false); // Отключение перемещения узлов
            }
        }
        view->setCursor(Qt::CrossCursor);                  // Установка курсора-крестика
    });

    bar->addSeparator();                                   // Разделитель


    QPushButton* execBtn = new QPushButton("Выполнить");
    execBtn->setObjectName("executeButton");  // Даём имя для поиска
        execBtn->setEnabled(false);  // Изначально неактивна
        bar->addWidget(execBtn);
        QObject::connect(execBtn, &QPushButton::clicked, this, &MainWindow::executeGraph);
        // Кнопка "Инструкция"
            QAction* instructionsAction = bar->addAction("Инструкция");
            QObject::connect(instructionsAction, &QAction::triggered, this, &MainWindow::showInstruction);

            bar->addSeparator();
}

// ==================== КОНТЕКСТНОЕ МЕНЮ И СОЗДАНИЕ УЗЛОВ ====================

void MainWindow::showContextMenu(const QPoint& pos) {
    if (scene->getMode() == DiagramScene::InsertLine) return; // В режиме связей контекстное меню недоступно
    if (scene->itemAt(view->mapToScene(pos), QTransform())) return; // Если клик на узле - не показываем меню

    menu_pos = view->mapToScene(pos);                    // Сохраняем позицию для создания узла
    QMenu menu;                                            // Создание контекстного меню
    QMenu* ioMenu = menu.addMenu("Ввод/Вывод");            // Подменю ввода/вывода
    ioMenu->addAction("Ввод", [this]() { createNode(NodeType::INPUT); });     // Создание узла ввода
    ioMenu->addAction("Вывод", [this]() { createNode(NodeType::OUTPUT); });   // Создание узла вывода
    QMenu* numMenu = menu.addMenu("Для чисел");            // Подменю числовых операций
    numMenu->addAction("Сумма", [this]() { createNode(NodeType::SUM); });       // Создание узла суммы
    numMenu->addAction("Медиана", [this]() { createNode(NodeType::MEDIAN); });  // Создание узла медианы
    numMenu->addAction("Среднее", [this]() { createNode(NodeType::AVERAGE); }); // Создание узла среднего
    QMenu* txtMenu = menu.addMenu("Для текста");           // Подменю текстовых операций
    txtMenu->addAction("Верхний регистр", [this]() { createNode(NodeType::TO_UPPER); }); // Создание узла верхнего регистра
    txtMenu->addAction("Нижний регистр", [this]() { createNode(NodeType::TO_LOWER); }); // Создание узла нижнего регистра
    menu.exec(view->viewport()->mapToGlobal(pos));         // Отображение меню
}

void MainWindow::createNode(NodeType type) {
    int id = graph.getNextId();                            // Получение следующего ID для узла
    VisualNode* node = new VisualNode(id, type);           // Создание визуального узла
    node->setPos(menu_pos);                              // Установка позиции в месте клика
    node->setFlag(QGraphicsItem::ItemIsMovable, true);     // Узел можно перемещать
    scene->addItem(node);                                  // Добавление узла на сцену
    addNode(node);                                      // Добавление узла в логический граф
    updateExecuteButton();
}

// ==================== УПРАВЛЕНИЕ ГРАФОМ ====================

void MainWindow::addNode(VisualNode* node) {
    graph.addNode(node->pos().x(), node->pos().y(), node->getNodeType()); // Добавление узла в логический граф
    Node* graphNode = graph.findNodeById(graph.getNextId() - 1);          // Поиск добавленного узла
    if (graphNode) {
        node->setNodeId(graphNode->nodeID);                // Сохранение ID в визуальном узле
        idToNode[graphNode->nodeID] = node;
        if (node->getNodeType() == NodeType::INPUT && !node->getInputData().isEmpty()) {
            graphNode->result = node->getInputData().toStdString(); // Сохранение данных ввода
            graphNode->calculated = true;
        }
    }
}

void MainWindow::removeNode(VisualNode* node) {
    if (!node->getNodeId())
            return;
    graph.deleteNode(node->getNodeId());
    idToNode.remove(node->getNodeId());
}

void MainWindow::addConnection(Arrow* arrow) {
    int fromId = arrow->startItem()->getNodeId();    // ← ID начального узла
    int toId = arrow->endItem()->getNodeId();        // ← ID конечного узла
    graph.addRelation(fromId, toId);                 // Добавление связи в логический граф
    arrowToIds[arrow] = qMakePair(fromId, toId);     // Сохранение связи для возможного удаления
    arrow->setZValue(-1000);                         // Стрелка под узлами
}

void MainWindow::removeConnection(Arrow* arrow) {
    if (!arrowToIds.contains(arrow))
        return;               // Если связи нет - выход
    auto conn = arrowToIds[arrow];                         // Получение ID узлов связи
    graph.deleteRelation(conn.first, conn.second);         // Удаление связи из логического графа
    arrowToIds.remove(arrow);                              // Удаление из маппинга
}

void MainWindow::onArrowCreated(Arrow* arrow) {
    arrow->setZValue(-1000);                               // Стрелка под узлами
    addConnection(arrow);                                  // Добавление связи в граф
    if (!graph.isDAG()) {                               // Проверка на циклы
        QMessageBox::warning(this, "Ошибка", "Эта связь создаст цикл в графе!"); // Сообщение об ошибке
        removeConnection(arrow);                           // Удаление связи
        arrow->startItem()->removeArrow(arrow);            // Удаление из начального узла
        arrow->endItem()->removeArrow(arrow);              // Удаление из конечного узла
        scene->removeItem(arrow);                          // Удаление со сцены
        delete arrow;                                      // Уничтожение стрелки
    }
    updateExecuteButton();
}


// ==================== УДАЛЕНИЕ ЭЛЕМЕНТОВ ====================
void MainWindow::deleteItem() {
    QList<QGraphicsItem*> selected = scene->selectedItems(); // Получение выделенных элементов
    for (QGraphicsItem* item : selected) {
        if (Arrow* arrow = qgraphicsitem_cast<Arrow*>(item)) { // Если выделена стрелка
            removeConnection(arrow);                       // Удаление связи из графа
            if (arrow->startItem())
                arrow->startItem()->removeArrow(arrow); // Удаление из начального узла
            if (arrow->endItem())
                arrow->endItem()->removeArrow(arrow);     // Удаление из конечного узла
            scene->removeItem(arrow);                      // Удаление со сцены
            delete arrow;                                  // Уничтожение стрелки
        } else if (VisualNode* node = qgraphicsitem_cast<VisualNode*>(item)) { // Если выделен узел
            for (Arrow* arrow : node->getArrows()) {       // Перебор всех стрелок узла
                removeConnection(arrow);                   // Удаление связи из графа
                scene->removeItem(arrow);                  // Удаление со сцены
                delete arrow;                              // Уничтожение стрелки
            }
            removeNode(node);                         // Удаление узла из графа
            scene->removeItem(node);                       // Удаление со сцены
            delete node;                                   // Уничтожение узла
        }
    }
    updateExecuteButton();
}


void MainWindow::executeGraph() {
    // Синхронизация: добавление всех узлов и стрелок в логический граф
    for (QGraphicsItem* item : scene->items()) {
        if (VisualNode* node = qgraphicsitem_cast<VisualNode*>(item)) {
            if (!graph.findNodeById(node->getNodeId())) addNode(node);
        } else if (Arrow* arrow = qgraphicsitem_cast<Arrow*>(item)) {
            if (!arrowToIds.contains(arrow)) addConnection(arrow);
        }
    }

    // Сохранение данных всех входных узлов перед выполнением
    QMap<int, QString> savedInputData;
    for (auto it = idToNode.begin(); it != idToNode.end(); ++it) {
        VisualNode* visNode = it.value();
        if (visNode->getNodeType() == NodeType::INPUT && visNode->isCalculated()) {
            savedInputData[it.key()] = visNode->getInputData();
        }
    }

    // Передача данных из визуальных узлов в логический граф
    for (auto it = idToNode.begin(); it != idToNode.end(); ++it) {
        Node* graphNode = graph.findNodeById(it.key());
        VisualNode* visNode = it.value();
        if (graphNode && visNode->getNodeType() == NodeType::INPUT && visNode->isCalculated()) {
            graphNode->result = visNode->getInputData().toStdString();
            graphNode->calculated = true;
        }
    }

    // Очистка данных узлов вывода перед выполнением
    for (auto it = idToNode.begin(); it != idToNode.end(); ++it) {
        if (it.value()->getNodeType() == NodeType::OUTPUT) {
            it.value()->clearOutputData();
        }
    }

    graph.execute();                                       // Выполнение логического графа

    // Восстановление данных входных узлов (они могли быть изменены при выполнении)
    for (auto it = savedInputData.begin(); it != savedInputData.end(); ++it) {
        Node* graphNode = graph.findNodeById(it.key());
        VisualNode* visNode = idToNode[it.key()];
        if (graphNode && visNode) {
            graphNode->result = it.value().toStdString();
            graphNode->calculated = true;
            visNode->setInputData(it.value());
        }
    }

    // Передача результатов из логического графа в визуальные узлы
    for (auto it = idToNode.begin(); it != idToNode.end(); ++it) {
        Node* graphNode = graph.findNodeById(it.key());
        VisualNode* visNode = it.value();
        if (graphNode && graphNode->calculated) {
            if (visNode->getNodeType() == NodeType::OUTPUT) {
                // Для узлов вывода: каждый результат от отдельного источника добавляется отдельно
                for (int prevId : graphNode->prev_nodes) {
                    Node* prev = graph.findNodeById(prevId);
                    if (prev && prev->calculated) {
                        visNode->addOutputData(QString::fromStdString(prev->result));
                    }
                }
            } else if (visNode->getNodeType() != NodeType::INPUT) {
                // Для остальных узлов: преобразование результата в число
                double val = QString::fromStdString(graphNode->result).toDouble();
                visNode->setResult(val);
            }
        }
    }

    // Сброс режима в "Узлы" после выполнения
    scene->setMode(DiagramScene::InsertNode);

    // Сброс состояния кнопок: включаем "Узлы", выключаем "Связи"
    if (node_button) {
        node_button->setChecked(true);                       // Кнопка "Узлы" становится нажатой
    }
    if (arrow_button) {
        arrow_button->setChecked(false);                       // Кнопка "Связи" становится отжатой
    }

    // Включаем перемещение для всех узлов
    for (QGraphicsItem* item : scene->items()) {
        if (VisualNode* node = qgraphicsitem_cast<VisualNode*>(item)) {
            node->setFlag(QGraphicsItem::ItemIsMovable, true);
        }
    }
    view->setCursor(Qt::ArrowCursor);

    QMessageBox::information(this, "Выполнение", "Граф успешно выполнен!");
}

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

void MainWindow::showInstruction() {
    QString fileName = "instruction.txt";
    QFile file(fileName);

    if (!file.exists()) {
        QMessageBox::warning(this, "Ошибка",
            "Файл instruction.txt не найден!\n"
            "Убедитесь, что файл находится в папке с программой.");
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл инструкции!");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    // Убираем BOM если есть
    if (data.size() >= 3 && data[0] == '\xEF' && data[1] == '\xBB' && data[2] == '\xBF') {
        data = data.mid(3);
    }

    QString content = QString::fromUtf8(data);

    QDialog* dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle("Инструкция");
    dialog->resize(600, 500);

    QVBoxLayout* layout = new QVBoxLayout(dialog);

    QTextEdit* textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setPlainText(content);
    textEdit->setFont(QFont("Consolas", 11));
    layout->addWidget(textEdit);

    QPushButton* closeButton = new QPushButton("Закрыть");
    layout->addWidget(closeButton);

    QObject::connect(closeButton, &QPushButton::clicked, dialog, &QDialog::close);

    dialog->show();
}

