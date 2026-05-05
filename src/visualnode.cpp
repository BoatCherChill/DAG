#include "visualnode.h"
#include "arrow.h"
#include "mainwindow.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QHBoxLayout>

// конструктор
VisualNode::VisualNode(int id, NodeType type, QGraphicsItem* parent)
    : QGraphicsEllipseItem(parent), nodeId(id), nodeType(type),
      calculated(false), value(0), outputDialogOpen(false)
{
    setRect(-35, -35, 70, 70);

    if (type == NodeType::INPUT)
        dataType = DataType::NUMBER;
    else if (type == NodeType::TO_UPPER || type == NodeType::TO_LOWER)
        dataType = DataType::TEXT;
    else
        dataType = DataType::NUMBER;


    setZValue(0);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    QColor color;
    if (type == NodeType::INPUT)
        color = QColor(200, 230, 255);
    else if (type == NodeType::OUTPUT)
        color = QColor(255, 230, 200);
    else if (type == NodeType::SUM || type == NodeType::MEDIAN || type == NodeType::AVERAGE)
        color = QColor(230, 255, 230);
    else
        color = QColor(255, 230, 255);
    setBrush(color);
    setPen(QPen(Qt::black, 2));
}


QString VisualNode::normalizeDataString(const QString& data) const {
    QString result = data;
    result.replace('\n', ' ');
    result.replace('\r', ' ');
    result.replace('\t', ' ');
    result = result.simplified();
    return result;
}

bool VisualNode::validateNumberData(const QString& data) {
    QString normalized = normalizeDataString(data);
    QStringList parts = normalized.split(' ', Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        bool ok;
        part.toDouble(&ok);
        if (!ok)
            return false;

    }
    return true;
}

void VisualNode::setInputData(const QString& data) {
    inputData = data;
    calculated = true;
    update();
}

void VisualNode::setResult(double val) {
    value = val;
    inputData = QString::number(val);
    calculated = true;
    update();
}

void VisualNode::addOutputData(const QString& data) {
    if (nodeType != NodeType::OUTPUT) return;
    outputDataList.append(data);
    calculated = true;
    value = outputDataList.size();
    update();
}

void VisualNode::showOutputDialog() {
    if (nodeType != NodeType::OUTPUT)
        return;
    if (outputDialogOpen)
        return;

    QDialog* dialog = new QDialog();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle("Результат");
    dialog->setMinimumSize(400, 300);
    dialog->setModal(true);

    QVBoxLayout* layout = new QVBoxLayout(dialog);

    QTextEdit* textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setPlainText(outputDataList.join("\n"));
    layout->addWidget(textEdit);

    QPushButton* closeButton = new QPushButton("Закрыть");
    layout->addWidget(closeButton);

    QObject::connect(closeButton, &QPushButton::clicked, dialog, &QDialog::close);
    QObject::connect(dialog, &QDialog::finished, [this]() {
        outputDialogOpen = false;
    });

    outputDialogOpen = true;
    dialog->exec();
}


void VisualNode::addArrow(Arrow* arrow) {
    if (!arrows.contains(arrow)) arrows.append(arrow);
}

void VisualNode::removeArrow(Arrow* arrow) {
    arrows.removeAll(arrow);
}

void VisualNode::removeArrows() {
    for (Arrow* arrow : arrows) {
        if (arrow->startItem()) arrow->startItem()->removeArrow(arrow);
        if (arrow->endItem()) arrow->endItem()->removeArrow(arrow);
        if (scene()) scene()->removeItem(arrow);
        delete arrow;
    }
    arrows.clear();
}

bool VisualNode::isCompatibleWith(VisualNode* other) const {
    if (!other)
        return false;
    if (this == other)
        return false;

    // OUTPUT может принимать данные от ЛЮБОГО узла
    if (nodeType == NodeType::OUTPUT)
        return true;
    if (other->nodeType == NodeType::OUTPUT)
        return true;

    // INPUT нельзя соединять с INPUT
    if (nodeType == NodeType::INPUT && other->nodeType == NodeType::INPUT)
        return false;

    // INPUT может подключаться только к совместимым типам
    if (nodeType == NodeType::INPUT)
        return dataType == other->dataType;

    // Текстовые операции требуют текстовый вход
    if (nodeType == NodeType::TO_UPPER || nodeType == NodeType::TO_LOWER)
        return other->dataType == DataType::TEXT;

    // Числовые операции требуют числовой вход
    if (nodeType == NodeType::SUM || nodeType == NodeType::MEDIAN || nodeType == NodeType::AVERAGE)
        return other->dataType == DataType::NUMBER;


    return dataType == other->dataType;
}


void VisualNode::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    if (nodeType == NodeType::INPUT) {

        bool hasConnections = !arrows.isEmpty();
        QDialog dialog;
        dialog.setWindowTitle("Ввод данных");
        dialog.setMinimumWidth(450);
        QVBoxLayout* layout = new QVBoxLayout(&dialog);

        QLabel* typeLabel = new QLabel("Тип данных:");
        layout->addWidget(typeLabel);

        QComboBox* typeCombo = new QComboBox();
        typeCombo->addItem("Числовой");
        typeCombo->addItem("Текстовый");

        if (dataType == DataType::TEXT)
            typeCombo->setCurrentIndex(1);
        else
            typeCombo->setCurrentIndex(0);

        if (hasConnections) {
           typeCombo->setEnabled(false);
           QLabel* warningLabel = new QLabel("Изменение типа недоступно, пока есть связи");
           warningLabel->setStyleSheet("color: grey; font-size: 10px;");
           layout->addWidget(warningLabel);
        }

        layout->addWidget(typeCombo);

        QLabel* label = new QLabel("Введите данные:");
        layout->addWidget(label);

        QTextEdit* edit = new QTextEdit;
        edit->setPlainText(inputData);
        edit->setMinimumHeight(120);
        layout->addWidget(edit);

        QPushButton* fileButton = new QPushButton("Загрузить из файла");
        layout->addWidget(fileButton);

        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttonBox);

        DataType currentDataType = dataType;

        QObject::connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [&currentDataType](int index) {
                if (index == 1)
                    currentDataType = DataType::TEXT;
                else
                    currentDataType = DataType::NUMBER;
            });

        QObject::connect(fileButton, &QPushButton::clicked, [edit]() {
            QString fileName = QFileDialog::getOpenFileName(nullptr, "Выберите файл", "", "*.txt");
            if (fileName.isEmpty()) return;
            if (!fileName.endsWith(".txt", Qt::CaseInsensitive)) {
                QMessageBox::warning(nullptr, "Ошибка", "Только .txt файлы!");
                return;
            }
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMessageBox::warning(nullptr, "Ошибка", "Не удалось открыть файл!");
                return;
            }
            QByteArray data = file.readAll();
            file.close();
            if (data.isEmpty()) {
                QMessageBox::warning(nullptr, "Ошибка", "Файл пуст!");
                return;
            }
            edit->setPlainText(QString::fromUtf8(data));
        });

        QObject::connect(buttonBox, &QDialogButtonBox::accepted, [this, edit, &dialog, &currentDataType]() {
            if (currentDataType == DataType::NUMBER) {
                if (!validateNumberData(edit->toPlainText())) {
                    QMessageBox::warning(&dialog, "Ошибка ввода", "Данные не соответствуют числовому типу!\nВведите числа через пробел.");
                    return;
                }
            }
            dataType = currentDataType;
            setInputData(edit->toPlainText());
            dialog.accept();
        });

        QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        dialog.exec();
    } else if (nodeType == NodeType::OUTPUT) {
        showOutputDialog();
    }
    QGraphicsEllipseItem::mouseDoubleClickEvent(event);
}

void VisualNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    QGraphicsEllipseItem::paint(painter, option, widget);

    QString title;
    switch(nodeType) {
        case NodeType::INPUT:
            title = "IN";
            break;
        case NodeType::OUTPUT:
            title = "OUT";
            break;
        case NodeType::SUM:
            title = "SUM";
            break;
        case NodeType::MEDIAN:
            title = "MED";
            break;
        case NodeType::AVERAGE:
            title = "AVG";
            break;
        case NodeType::TO_UPPER:
            title = "UP";
            break;
        case NodeType::TO_LOWER:
            title = "LOW";
            break;
    }

    if (nodeType == NodeType::INPUT) {
            QFont smallFont = painter->font();
            smallFont.setPointSize(7);
            painter->setFont(smallFont);


            QString typeText;
            if (dataType == DataType::NUMBER)
                typeText = "NUM";
            else
                typeText = "TEXT";

            painter->drawText(rect().adjusted(0, 25, 0, 0), Qt::AlignCenter, typeText);
        }

    painter->save();
    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setPointSize(11);
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(rect(), Qt::AlignCenter, title);
    painter->restore();
}

QVariant VisualNode::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionChange) {
        for (Arrow* arrow : arrows)
            arrow->updatePosition();
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}


void VisualNode::cleanResults(){
    inputData.clear();
    outputDataList.clear();
    calculated = false;
    value = 0;
    update();
}
