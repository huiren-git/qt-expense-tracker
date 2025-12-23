#include "daydetailwidget.h"
#include "recordeditdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

DayDetailWidget::DayDetailWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void DayDetailWidget::setupUI()
{
    setStyleSheet("QWidget { background-color: #f0f4f8; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    setupHeader();
    mainLayout->addLayout(headerLayout);

    setupTable();
    mainLayout->addWidget(recordsTable);
}

void DayDetailWidget::setupHeader()
{
    headerLayout = new QHBoxLayout();

    backButton = new QPushButton("返回");
    backButton->setFixedSize(80, 35);
    backButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
    );
    connect(backButton, &QPushButton::clicked, this, &DayDetailWidget::onBackClicked);
    headerLayout->addWidget(backButton);

    addButton = new QPushButton("新增");
    addButton->setFixedSize(80, 35);
    addButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
    );
    connect(addButton, &QPushButton::clicked, this, &DayDetailWidget::onAddClicked);
    headerLayout->addWidget(addButton);

    headerLayout->addStretch();

    dateCardLabel = new QLabel();
    dateCardLabel->setFixedSize(250, 80);
    dateCardLabel->setStyleSheet(
        "QLabel { "
        "background-color: white; "
        "border: 1px solid #d0d8e0; "
        "border-radius: 10px; "
        "padding: 15px; "
        "font-size: 16px; "
        "color: #333; "
        "}"
    );
    dateCardLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(dateCardLabel);
}

void DayDetailWidget::setupTable()
{
    recordsTable = new QTableWidget(0, 9, this);
    recordsTable->setHorizontalHeaderLabels({
        "操作", "交易时间", "交易类型", "交易分类",
        "交易方式", "交易对方", "交易内容", "备注", "订单号"
    });

    recordsTable->horizontalHeader()->setStretchLastSection(true);
    recordsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recordsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    recordsTable->setStyleSheet(
        "QTableWidget { "
        "background-color: white; "
        "border: 1px solid #d0d8e0; "
        "gridline-color: #e0e8f0; "
        "}"
        "QTableWidget::item { "
        "padding: 5px; "
        "}"
        "QTableWidget::item:selected { "
        "background-color: #3b6ea5; "
        "color: white; "
        "}"
    );

    recordsTable->setColumnWidth(0, 120);  // 操作列
    recordsTable->setColumnWidth(1, 150);  // 交易时间
    recordsTable->setColumnWidth(2, 80);   // 交易类型
    recordsTable->setColumnWidth(3, 100);   // 交易分类
    recordsTable->setColumnWidth(4, 80);    // 交易方式
}

void DayDetailWidget::loadDayData(const QString &date)
{
    currentDate = date;

    // 创建测试数据
        QJsonObject testData;
        testData["operation"] = true;
        testData["dailyIncome"] = 500.00;
        testData["dailyExpense"] = 2600.00;

        QJsonArray records;

        QJsonObject record1;
        record1["id"] = 1001;
        record1["transactionDate"] = date + " 10:30:00";
        record1["amount"] = 2000.00;
        record1["transactionType"] = "支出";
        record1["category"] = "日用百货";
        record1["transactionMethod"] = "支付宝";
        record1["counterparty"] = "XX电商平台";
        record1["productName"] = "智能手机";
        record1["remark"] = "分期购买";
        record1["sourceId"] = "202401152030001234567890";
        records.append(record1);

        QJsonObject record2;
        record2["id"] = 1002;
        record2["transactionDate"] = date + " 18:00:00";
        record2["amount"] = 600.00;
        record2["transactionType"] = "支出";
        record2["category"] = "餐饮美食";
        record2["transactionMethod"] = "现金";
        record2["counterparty"] = "麦当劳";
        record2["productName"] = "晚餐";
        record2["remark"] = "";
        record2["sourceId"] = QJsonValue();
        records.append(record2);

        testData["records"] = records;

        updateDayData(testData);

    // 更新日期卡片
    //dateCardLabel->setText(QString("日期: %1\n交易笔数: 0\n总金额: ￥0.00")
        //.arg(date));

    // TODO: 调用后端接口获取数据
    // QJsonObject request;
    // request["date"] = date;
    // emit queryDayRecords(request);
}

void DayDetailWidget::updateDayData(const QJsonObject &data)
{
    recordsTable->setRowCount(0);

    double dailyIncome = data["dailyIncome"].toDouble();
    double dailyExpense = data["dailyExpense"].toDouble();
    int recordCount = data["records"].toArray().size();

    dateCardLabel->setText(QString("日期: %1\n交易笔数: %2\n总金额: ￥%3")
        .arg(currentDate)
        .arg(recordCount)
        .arg(dailyIncome + dailyExpense, 0, 'f', 2));

    QJsonArray records = data["records"].toArray();
    for (const QJsonValue &value : records) {
        QJsonObject record = value.toObject();
        int row = recordsTable->rowCount();
        recordsTable->insertRow(row);

        // 操作列：删除和编辑按钮
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(5, 2, 5, 2);
        actionLayout->setSpacing(5);

        QPushButton *deleteBtn = new QPushButton("删除");
        deleteBtn->setFixedSize(50, 25);
        deleteBtn->setStyleSheet(
            "QPushButton { "
            "background-color: #e74c3c; "
            "color: white; "
            "border: none; "
            "border-radius: 3px; "
            "font-size: 12px; "
            "}"
            "QPushButton:hover { background-color: #c0392b; }"
        );
        connect(deleteBtn, &QPushButton::clicked, this, [this, row]() {
            onDeleteClicked(row);
        });

        QPushButton *editBtn = new QPushButton("编辑");
        editBtn->setFixedSize(50, 25);
        editBtn->setStyleSheet(
            "QPushButton { "
            "background-color: #3b6ea5; "
            "color: white; "
            "border: none; "
            "border-radius: 3px; "
            "font-size: 12px; "
            "}"
            "QPushButton:hover { background-color: #4a7fb8; }"
        );
        connect(editBtn, &QPushButton::clicked, this, [this, row]() {
            onEditClicked(row);
        });

        actionLayout->addWidget(deleteBtn);
        actionLayout->addWidget(editBtn);
        recordsTable->setCellWidget(row, 0, actionWidget);

        // 其他列
        recordsTable->setItem(row, 1, new QTableWidgetItem(record["transactionDate"].toString()));
        recordsTable->setItem(row, 2, new QTableWidgetItem(record["transactionType"].toString()));
        recordsTable->setItem(row, 3, new QTableWidgetItem(record["category"].toString()));
        recordsTable->setItem(row, 4, new QTableWidgetItem(record["transactionMethod"].toString()));
        recordsTable->setItem(row, 5, new QTableWidgetItem(record["counterparty"].toString()));
        recordsTable->setItem(row, 6, new QTableWidgetItem(record["productName"].toString()));
        recordsTable->setItem(row, 7, new QTableWidgetItem(record["remark"].toString()));
        QString sourceId = record["sourceId"].isNull() ? "" : record["sourceId"].toString();
        recordsTable->setItem(row, 8, new QTableWidgetItem(sourceId));
    }
}

void DayDetailWidget::onAddClicked()
{
    RecordEditDialog *dialog = new RecordEditDialog(this, false);
    if (dialog->exec() == QDialog::Accepted) {
        QJsonObject record = dialog->getRecordData();
        // TODO: 发送新增请求
        emit addRecordRequested();
    }
}

void DayDetailWidget::onEditClicked(int row)
{
    // 从表格获取记录数据
    QJsonObject record;
    record["transactionDate"] = recordsTable->item(row, 1)->text();
    record["transactionType"] = recordsTable->item(row, 2)->text();
    record["category"] = recordsTable->item(row, 3)->text();
    record["transactionMethod"] = recordsTable->item(row, 4)->text();
    record["counterparty"] = recordsTable->item(row, 5)->text();
    record["productName"] = recordsTable->item(row, 6)->text();
    record["remark"] = recordsTable->item(row, 7)->text();
    record["sourceId"] = recordsTable->item(row, 8)->text();

    RecordEditDialog *dialog = new RecordEditDialog(this, true);
    dialog->setRecordData(record);
    if (dialog->exec() == QDialog::Accepted) {
        QJsonObject updatedRecord = dialog->getRecordData();
        // TODO: 发送更新请求
        emit editRecordRequested(updatedRecord);
    }
}

void DayDetailWidget::onDeleteClicked(int row)
{
    int ret = QMessageBox::question(this, "确认删除", "确定要删除这条记录吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        // TODO: 获取记录ID并发送删除请求
        // qint64 id = ...;
        // emit deleteRecordRequested(id);
    }
}

void DayDetailWidget::onBackClicked()
{
    emit backToMainView();
}
