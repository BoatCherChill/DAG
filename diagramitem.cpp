#include "diagramitem.h"
#include "arrow.h"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QtMath>

DiagramItem::DiagramItem(DiagramType diagramType, QMenu *contextMenu, QGraphicsItem *parent)
    : QGraphicsPolygonItem(parent), myDiagramType(diagramType), myContextMenu(contextMenu), myValue(0), calculated(false), nodeId(-1), outputDialogOpen(false)
{
    // Создаем полигон в зависимости от типа
    switch (myDiagramType) {
        case Input:
        case Output:
            myPolygon << QPointF(-100, -80) << QPointF(100, -80) << QPointF(100, 80) << QPointF(-100, 80) << QPointF(-100, -80);
            break;

        case Sum:
        case Median:
        case Average:
        case Text_Concat:
        case To_Lower:
        case To_Upper:
            for (int i = 0; i < 360; i += 10) {
                double rad = i * M_PI / 180.0;
                myPolygon << QPointF(80 * qCos(rad), 60 * qSin(rad));
            }
            break;
        default:
            break;
    }

    setPolygon(myPolygon);

    // Устанавливаем тип данных в зависимости от типа узла (только один раз!)
    switch (myDiagramType) {
        case Input:
            myDataType = DataType::NUMBER;
            break;
        case Output:
            break;
        case Text_Concat:
        case To_Lower:
        case To_Upper:
            myDataType = DataType::TEXT;
            break;
        default:  // Sum, Median, Average
            myDataType = DataType::NUMBER;
            break;
    }

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    /*QColor color;
    switch (myDiagramType) {
        case Input: color = QColor(200, 230, 255); break;
        case Output: color = QColor(255, 230, 200); break;
        case Sum: case Median: case Average: case Text_Concat: case To_Lower: case To_Upper:
            color = QColor(230, 255, 230); break;
    }
    setBrush(color);*/
}

void DiagramItem::removeArrow(Arrow *arrow)
{
    arrows.removeAll(arrow);
}

void DiagramItem::removeArrows()
{
    const auto arrowsCopy = arrows;
    for (Arrow *arrow : arrowsCopy) {
        arrow->startItem()->removeArrow(arrow);
        arrow->endItem()->removeArrow(arrow);
        scene()->removeItem(arrow);
        delete arrow;
    }
}

void DiagramItem::addArrow(Arrow *arrow)
{
    arrows.append(arrow);
}

void DiagramItem::setValue(double value)
{
    myValue = value;
    calculated = true;
    update();
}

void DiagramItem::setTextData(const QString &text)
{
    myTextData = text;
    myDataType = DataType::TEXT;
    calculated = true;

    myInputArray.clear();
    QStringList parts = text.split(' ', Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        bool ok;
        double num = part.toDouble(&ok);
        if (ok) {
            myInputArray.append(num);
        }
    }

    if (!myInputArray.isEmpty()) {
        myValue = myInputArray.first();
    }

    update();
}

void DiagramItem::setInputArray(const QList<double>& array)
{
    myInputArray = array;
    if (!array.isEmpty()) {
        myValue = array.first();
        calculated = true;
    }
    update();
}

void DiagramItem::addOutputData(const QString &data)
{
    if (myDiagramType != Output) return;

    // Если данные выглядят как число, можно добавить без изменений
    // Или можно форматировать для лучшего отображения
    outputDataList.append(data);
    calculated = true;

    // Обновляем отображение на узле - показываем количество строк
    myValue = outputDataList.size();
    update();
}

void DiagramItem::clearOutputData()
{
    if (myDiagramType != Output) return;

    outputDataList.clear();
    calculated = false;
    myValue = 0;
    update();
}

void DiagramItem::showOutputDialog()
{
    if (myDiagramType != Output) return;

    if (outputDialogOpen) return;  // Диалог уже открыт

    QDialog *dialog = new QDialog();
    dialog->setWindowTitle(QString("Вывод узла %1").arg(nodeId));
    dialog->setMinimumSize(500, 400);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QLabel *label = new QLabel(QString("Результаты выполнения (всего строк: %1):").arg(outputDataList.size()));
    layout->addWidget(label);

    QTextEdit *textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 10));

    // Заполняем текстовое поле
    QString content;
    for (int i = 0; i < outputDataList.size(); ++i) {
        content += QString("%1: %2\n").arg(i + 1).arg(outputDataList[i]);
    }
    textEdit->setPlainText(content);
    layout->addWidget(textEdit);

    QPushButton *closeButton = new QPushButton("Закрыть");
    QPushButton *clearButton = new QPushButton("Очистить");
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);

    // Подключаем кнопки
    QObject::connect(closeButton, &QPushButton::clicked, [dialog]() {
        dialog->close();
    });

    QObject::connect(clearButton, &QPushButton::clicked, [this, textEdit]() {
        clearOutputData();
        textEdit->clear();
        // Обновляем заголовок
        QLabel *lbl = qobject_cast<QLabel*>(textEdit->parentWidget()->layout()->itemAt(0)->widget());
        if (lbl) {
            lbl->setText(QString("Результаты выполнения (всего строк: 0):"));
        }
    });

    // Удаляем диалог при закрытии
    QObject::connect(dialog, &QDialog::finished, [this, dialog]() {
        outputDialogOpen = false;
        dialog->deleteLater();
    });

    outputDialogOpen = true;
    dialog->show();
}

void DiagramItem::setInputData(const QString &data)
{
    myInputData = data;

    if (myDataType == DataType::TEXT) {
        myTextData = data;
        myInputArray.clear();

        QStringList parts = data.split(' ', Qt::SkipEmptyParts);
        for (const QString& part : parts) {
            bool ok;
            double num = part.toDouble(&ok);
            if (ok) {
                myInputArray.append(num);
            }
        }

        myValue = myInputArray.isEmpty() ? 0 : myInputArray.first();
        calculated = true;
    } else {
        if (data.contains(' ')) {
            QStringList parts = data.split(' ', Qt::SkipEmptyParts);
            myInputArray.clear();

            for (const QString& part : parts) {
                bool ok;
                double num = part.toDouble(&ok);
                if (ok) {
                    myInputArray.append(num);
                } else {
                    QMessageBox::warning(nullptr, "Ошибка",
                                       QString("Некорректное число: %1").arg(part));
                    return;
                }
            }

            if (!myInputArray.isEmpty()) {
                myValue = myInputArray.first();
                calculated = true;
            }
        } else {
            bool ok;
            double num = data.toDouble(&ok);
            if (ok) {
                myValue = num;
                myInputArray.clear();
                myInputArray.append(num);
                calculated = true;
            } else if (!data.isEmpty()) {
                QMessageBox::warning(nullptr, "Ошибка", "Введите корректное число!");
                return;
            }
        }
    }

    update();
}

void DiagramItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (myDiagramType == Input) {
        QDialog dialog;
        dialog.setWindowTitle("Ввод данных");
        QVBoxLayout *layout = new QVBoxLayout(&dialog);

        QLabel *typeLabel = new QLabel("Тип данных:");
        layout->addWidget(typeLabel);

        QComboBox *typeCombo = new QComboBox();
        typeCombo->addItem("Числовой");
        typeCombo->addItem("Текстовый");
        typeCombo->setCurrentIndex(myDataType == DataType::TEXT ? 1 : 0);
        layout->addWidget(typeCombo);

        QLabel *label = new QLabel("Введите данные:");
        layout->addWidget(label);

        QLabel *hintLabel = new QLabel("Числовой режим: число или числа через пробел\nТекстовый режим: любой текст");
        hintLabel->setStyleSheet("color: gray; font-size: 10px;");
        layout->addWidget(hintLabel);

        QLineEdit *lineEdit = new QLineEdit;
        lineEdit->setText(myInputData);
        layout->addWidget(lineEdit);

        QPushButton *fileButton = new QPushButton("Загрузить из файла");
        layout->addWidget(fileButton);

        QPushButton *okButton = new QPushButton("OK");
        QPushButton *cancelButton = new QPushButton("Отмена");
        QHBoxLayout *buttonLayout = new QHBoxLayout;
        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);
        layout->addLayout(buttonLayout);

        QObject::connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [&](int index) {
                myDataType = index == 1 ? DataType::TEXT : DataType::NUMBER;
            });

        QObject::connect(fileButton, &QPushButton::clicked, [&]() {
            QString fileName = QFileDialog::getOpenFileName(nullptr, "Выберите файл", "", "Текстовые файлы (*.txt);;Все файлы (*)");
            if (!fileName.isEmpty()) {
                QFile file(fileName);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream stream(&file);
                    QString content = stream.readAll().trimmed();
                    lineEdit->setText(content);
                    file.close();
                }
            }
        });

        QObject::connect(okButton, &QPushButton::clicked, [&, lineEdit, &dialog]() {
            setInputData(lineEdit->text());
            dialog.accept();
        });

        QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

        dialog.exec();
    }
    else if (myDiagramType == Output) {
        showOutputDialog();  // Показываем диалог с результатами
    }
    else {
        QGraphicsPolygonItem::mouseDoubleClickEvent(event);
    }
}

void DiagramItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsPolygonItem::paint(painter, option, widget);
    painter->save();
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(Qt::NoBrush);

    QFont font = painter->font();
    font.setPointSize(12);
    painter->setFont(font);

    QString text;
    switch (myDiagramType) {
        case Input: {
            if (calculated) {
                if (myDataType == DataType::TEXT) {
                    text = myTextData.left(15) + (myTextData.length() > 15 ? "..." : "");
                } else if (myInputArray.size() > 1) {
                    text = QString("[%1] %2...").arg(myInputArray.size()).arg(myInputArray.first());
                } else {
                    text = QString::number(myValue);
                }
            } else {
                text = myDataType == DataType::TEXT ? "Ввод (текст)" : "Ввод (число)";
            }
            break;
        }
        case Output: {
            if (calculated && !outputDataList.isEmpty()) {
                text = QString("Вывод\n(%1 стр.)").arg(outputDataList.size());
            } else {
                text = "Вывод";
            }
            break;
        }
        case Sum: text = "Сумма"; break;
        case Median: text = "Медиана"; break;
        case Average: text = "Среднее"; break;
        case Text_Concat: text = "Объединение"; break;
        case To_Lower: text = "К нижнему регистру"; break;
        case To_Upper: text = "К верхнему регистру"; break;
        default:
            if (calculated && myDiagramType != Input && myDiagramType != Output)
                text = QString::number(myValue);
            break;
    }

    if (!text.isEmpty()) {
        QRectF rect = boundingRect();
        painter->drawText(rect, Qt::AlignCenter, text);
    }

    painter->restore();
}

void DiagramItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);

    if (myDiagramType == Input) {
        QMenu menu;
        QAction *inputAction = menu.addAction("Ввести данные");
        QAction *fileAction = menu.addAction("Загрузить из файла");
        QAction *selectedAction = menu.exec(event->screenPos());

        if (selectedAction == inputAction) {
            bool ok;
            double value = QInputDialog::getDouble(nullptr, "Ввод числа", "Введите число:", myValue, -999999, 999999, 2, &ok);
            if (ok) {
                setValue(value);
                myInputData = QString::number(value);
            }
        } else if (selectedAction == fileAction) {
            QString fileName = QFileDialog::getOpenFileName(nullptr, "Выберите файл с данными", "", "Текстовые файлы (*.txt);;Все файлы (*)");
            if (!fileName.isEmpty()) {
                QFile file(fileName);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream stream(&file);
                    QString content = stream.readAll().trimmed();
                    bool ok;
                    double value = content.toDouble(&ok);
                    if (ok) {
                        setValue(value);
                        myInputData = content;
                    } else {
                        QMessageBox::warning(nullptr, "Ошибка", "Файл не содержит корректное число!");
                    }
                    file.close();
                }
            }
        }
    }
    else if (myDiagramType == Output) {
        QMenu menu;
        QAction *showAction = menu.addAction("Показать результаты");
        QAction *clearAction = menu.addAction("Очистить результаты");
        QAction *selectedAction = menu.exec(event->screenPos());

        if (selectedAction == showAction) {
            showOutputDialog();
        } else if (selectedAction == clearAction) {
            clearOutputData();
        }
    }
    else {
        myContextMenu->exec(event->screenPos());
    }
}

QVariant DiagramItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        for (Arrow *arrow : qAsConst(arrows))
            arrow->updatePosition();
    }
    return value;
}

bool DiagramItem::isCompatibleWith(DiagramItem *other) const
{
    if (!other || this == other) return false;

    // OUTPUT может принимать данные от ЛЮБОГО узла
    if (myDiagramType == Output || other->myDiagramType == Output) {
        return true;
    }

    // INPUT подключается только к совместимым типам
    if (myDiagramType == Input) {
        return myDataType == other->getDataType();
    }

    // Текстовые операции
    if (myDiagramType == Text_Concat || myDiagramType == To_Lower || myDiagramType == To_Upper) {
        return other->getDataType() == DataType::TEXT;
    }

    // Числовые операции
    if (myDiagramType == Sum || myDiagramType == Median || myDiagramType == Average) {
        return other->getDataType() == DataType::NUMBER;
    }

    return myDataType == other->getDataType();
}
