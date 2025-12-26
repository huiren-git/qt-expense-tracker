#include "daydetailwidget.h"
#include "recordeditdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSqlQuery>
#include <QDebug>
#include "../db/database_manager.h"
#include "weekviewwidget.h"
#include "monthviewwidget.h"
#include "yearviewwidget.h"

DayDetailWidget::DayDetailWidget(QWidget *parent)
    : QWidget(parent)
{
setupUI();
}

void DayDetailWidget::setupUI()
{
setStyleSheet("QWidget { background-color: #f5f8fb; }");

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
recordsTable = new QTableWidget(0, 10, this);
recordsTable->setHorizontalHeaderLabels({
    "操作", "交易时间","交易金额", "交易类型", "交易分类",
    "交易方式", "交易对方", "交易内容", "备注", "订单号"
});

recordsTable->verticalHeader()->setVisible(false);
// 增加行高：从默认的约 30 增加到 45 或 50
recordsTable->verticalHeader()->setDefaultSectionSize(48);
// 确保内容垂直居中
recordsTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
//顺便微调表头高度，使其更协调
recordsTable->horizontalHeader()->setFixedHeight(45);
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
    "QHeaderView::section { "
            "    background-color: #1e3a5f; " // 深蓝色背景
            "    color: white; "              // 白色文字
            "    padding: 4px; "
            "    border: 1px solid #ffffff; " // 表头单元格之间的白色边框
            "    font-weight: bold; "
            "}"
);

recordsTable->setColumnWidth(0, 120);  // 操作列
recordsTable->setColumnWidth(1, 150);  // 交易时间
recordsTable->setColumnWidth(2, 100);  // 交易金额
recordsTable->setColumnWidth(3, 80);   // 交易类型
recordsTable->setColumnWidth(4, 100);   // 交易分类
recordsTable->setColumnWidth(5, 80);    // 交易方式
}

void DayDetailWidget::loadDayData(const QString &date)
{
currentDate = date;

//初始配置，打开数据库
DatabaseManager &db = DatabaseManager::instance();
if(!db.isReady()){
    if(!db.openDatabase()){
        qDebug() << "载入单日单日记录时数据库开启失败";
        return;
    }
}
QSqlQuery query;

// 载入数据

    QJsonObject testData;
    testData["operation"] = true;
    query = db.getTotalRecordsByDay(date);
    query.next();
    double income = query.value(1).toDouble();
    double expense = query.value(0).toDouble();
    testData["dailyIncome"] = income;
    testData["dailyExpense"] = expense;

    QJsonArray records;
    QJsonObject record1;
    query = db.getRecordsByDay(date);
    while(query.next()){
        record1["id"] = query.value(0).toInt();
        record1["transactionDate"] = query.value(1).toString();
        record1["amount"] = query.value(5).toDouble();
        if(query.value(6).toString()=="income"){
            record1["transactionType"] = "收入";
        }
        else{
            record1["transactionType"] = "支出";
        }
        switch(query.value(7).toInt()){
            case 1:record1["category"] = "餐饮美食";break;
            case 2:record1["category"] = "服饰装扮";break;
            case 3:record1["category"] = "日用百货";break;
            case 4:record1["category"] = "家居家装";break;
            case 5:record1["category"] = "数码电器";break;
            case 6:record1["category"] = "运动户外";break;
            case 7:record1["category"] = "美容美发";break;
            case 8:record1["category"] = "母婴亲子";break;
            case 9:record1["category"] = "宠物";break;
            case 10:record1["category"] = "交通出行";break;
            case 11:record1["category"] = "爱车养车";break;
            case 12:record1["category"] = "住房物业";break;
            case 13:record1["category"] = "酒店旅游";break;
            case 14:record1["category"] = "文化休闲";break;
            case 15:record1["category"] = "教育培训";break;
            case 16:record1["category"] = "医疗健康";break;
            case 17:record1["category"] = "生活服务";break;
            case 18:record1["category"] = "公共服务";break;
            case 19:record1["category"] = "商业服务";break;
            case 20:record1["category"] = "公益捐赠";break;
            case 21:record1["category"] = "互助保障";break;
            case 22:record1["category"] = "投资理财";break;
            case 23:record1["category"] = "保险";break;
            case 24:record1["category"] = "信用借还";break;
            case 25:record1["category"] = "充值缴费";break;
            case 26:record1["category"] = "其他";break;
            case 27:record1["category"] = "收入";break;
            case 28:record1["category"] = "转账红包";break;
            case 29:record1["category"] = "亲友代付";break;
            case 30:record1["category"] = "账户存取";break;
            case 31:record1["category"] = "退款";break;
            case 32:record1["category"] = "其他";break;
            default:record1["category"] = "其他";break;
        }

        if(query.value(8).toInt()==1){
            record1["transactionMethod"] = "支付宝";
        }
        else if(query.value(8).toInt()==2){
            record1["transactionMethod"] = "现金";
        }
        else{
            record1["transactionMethod"] = "其他";
        }
        record1["counterparty"] = query.value(9).toString();
        record1["productName"] = query.value(10).toString();
        record1["remark"] = query.value(11).toString();
        record1["sourceId"] = query.value(12).toString();
        records.append(record1);
    }

    testData["records"] = records;

    updateDayData(testData);

// 更新日期卡片
//dateCardLabel->setText(QString("日期: %1\n交易笔数: 0\n总金额: ￥0.00")
    //.arg(date));

// TODO: 调用后端接口获取数据

//currentDate = date;

// 构造 API 2.2.4 请求

//QJsonObject request;
//request["date"] = date;

// 发送给后台
//emit queryDayRecords(request);
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
    actionLayout->setContentsMargins(2, 2, 5, 2);
    actionLayout->setSpacing(4);

    QPushButton *deleteBtn = new QPushButton("删除");
    deleteBtn->setFixedSize(45, 24);
    deleteBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #e74c3c; "
        "color: white; "
        "border: none; "
        "border-radius: 3px; "
        "font-size: 12px; "
        "padding: 0px; "
        "}"
        "QPushButton:hover { background-color: #c0392b; }"
    );
    connect(deleteBtn, &QPushButton::clicked, this, [this, row]() {
        onDeleteClicked(row);
    });

    QPushButton *editBtn = new QPushButton("编辑");
    editBtn->setFixedSize(45, 24);
    editBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 3px; "
        "font-size: 12px; "
        "padding: 0px; "
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
    double amountValue = record["amount"].toDouble(); // 获取数值
    QTableWidgetItem *amountItem = new QTableWidgetItem(QString::number(amountValue, 'f', 2)); // 格式化为2位小数的字符串
    recordsTable->setItem(row, 2, amountItem);
    recordsTable->setItem(row, 3, new QTableWidgetItem(record["transactionType"].toString()));
    recordsTable->setItem(row, 4, new QTableWidgetItem(record["category"].toString()));
    recordsTable->setItem(row, 5, new QTableWidgetItem(record["transactionMethod"].toString()));
    recordsTable->setItem(row, 6, new QTableWidgetItem(record["counterparty"].toString()));
    recordsTable->setItem(row, 7, new QTableWidgetItem(record["productName"].toString()));
    recordsTable->setItem(row, 8, new QTableWidgetItem(record["remark"].toString()));
    QString sourceId = record["sourceId"].isNull() ? "" : record["sourceId"].toString();
    recordsTable->setItem(row, 9, new QTableWidgetItem(sourceId));
}
}

void DayDetailWidget::onAddClicked()
{
RecordEditDialog *dialog = new RecordEditDialog(this, false);
if (dialog->exec() == QDialog::Accepted) {
    QJsonObject record = dialog->getRecordData();
    // TODO: 发送新增请求
    qDebug() << "开始插入";
    DatabaseManager &db=DatabaseManager::instance();
    if(!db.isReady()){
        if(!db.openDatabase()){
            qDebug() << "插入数据时数据库开启失败";
            return;
        }
    }
    db.addRecord(record["amount"].toDouble(), record["transactionType"].toString(), record["transactionDate"].toString(),
                 record["category"].toInt(), record["transactionMethod"].toInt(), record["counterparty"].toString(), record["productName"].toString(),
                 record["sourceId"].toString(),record["remark"].toString());

    emit addRecordRequested(record);
    emit dataChanged();  // 发出数据变化信号
    loadDayData(currentDate);
}
}

void DayDetailWidget::onEditClicked(int row)
{
// 从表格获取记录数据
QJsonObject record;
record["transactionDate"] = recordsTable->item(row, 1)->text();
record["amount"] = recordsTable->item(row, 2)->text();
record["transactionType"] = recordsTable->item(row, 3)->text();
record["category"] = recordsTable->item(row, 4)->text();
record["transactionMethod"] = recordsTable->item(row, 5)->text();
record["counterparty"] = recordsTable->item(row, 6)->text();
record["productName"] = recordsTable->item(row, 7)->text();
record["remark"] = recordsTable->item(row, 8)->text();
record["sourceId"] = recordsTable->item(row, 9)->text();

RecordEditDialog *dialog = new RecordEditDialog(this, true);
dialog->setRecordData(record);
if (dialog->exec() == QDialog::Accepted) {
        QJsonObject revisedData = dialog->getRecordData();

        QJsonObject request;
        request["transactionDate"] = record["transactionDate"]; // 原始主键
        request["revised"] = revisedData; // 修改后的内容
        DatabaseManager &db=DatabaseManager::instance();
        if(!db.isReady()){
            if(!db.openDatabase()){
                qDebug() << "更新数据时数据库开启失败";
                return;
            }
        }
        int id=-1;
        if(recordsTable->item(row, 3)->text()=="支出"){
            id = db.getExpenseBillIdByDate(recordsTable->item(row, 1)->text());
        }
        else{
            id = db.getIncomeBillIdByDate(recordsTable->item(row, 1)->text());
        }
        db.updateRecord(id, revisedData["amount"].toDouble(), revisedData["transactionType"].toString(), revisedData["transactionDate"].toString(),
                 revisedData["category"].toInt(), revisedData["transactionMethod"].toInt(), revisedData["counterparty"].toString(),
                        revisedData["productName"].toString(), revisedData["sourceId"].toString(),revisedData["remark"].toString());

        emit editRecordRequested(request);
        emit dataChanged();  // 发出数据变化信号
        loadDayData(currentDate);
    }
}

void DayDetailWidget::onDeleteClicked(int row)
{
QMessageBox msgBox(this);
msgBox.setWindowTitle("确认删除");
msgBox.setText("确定要删除这条记录吗？");
msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
msgBox.setButtonText(QMessageBox::Ok, "确认");
msgBox.setButtonText(QMessageBox::Cancel, "取消");

// 1. 核心布局美化：压缩间距，设置背景
msgBox.setStyleSheet(
    "QMessageBox { "
    "  background-color: white; "
    "}"
    "QLabel { "
    "  color: #333; "
    "  font-size: 14px; "
    "  min-height: 50px; "
    "  padding-top: 10px; "
    "  qproperty-alignment: 'AlignCenter'; "
    "}"
    "QWidget { "
    "  text-alignment: center; "   /* 强制内部组件对齐 */
    "}"
);

// 2. 缩小确认按钮 (红色圆角矩形)
QPushButton *okBtn = qobject_cast<QPushButton*>(msgBox.button(QMessageBox::Ok));
if (okBtn) {
    okBtn->setFixedSize(65, 28);   // 进一步缩小尺寸
    okBtn->setStyleSheet(
        "QPushButton { "
        "  background-color: #e74c3c; "
        "  color: white; "
        "  border-radius: 10px; "      /* 高度 28 的一半实现圆角 */
        "  font-size: 12px; "
        "  border: none; "
        "  margin-right: 5px; "        /* 与取消按钮拉开一点距离 */
        "}"
        "QPushButton:hover { background-color: #c0392b; }"
    );
}

// 3. 缩小取消按钮 (浅蓝色圆角矩形)
QPushButton *cancelBtn = qobject_cast<QPushButton*>(msgBox.button(QMessageBox::Cancel));
if (cancelBtn) {
    cancelBtn->setFixedSize(65, 28); // 与确认按钮尺寸一致
    cancelBtn->setStyleSheet(
        "QPushButton { "
        "  background-color: #3b6ea5; "
        "  color: white; "
        "  border-radius: 10px; "
        "  font-size: 12px; "
        "  border: none; "
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
    );
}

// 4. 执行并处理逻辑
if (msgBox.exec() == QMessageBox::Ok) {
    QString transDate = recordsTable->item(row, 1)->text();
    QJsonObject request;
    request["transactionDate"] = transDate;
    DatabaseManager &db=DatabaseManager::instance();
    if(!db.isReady()){
        if(!db.openDatabase()){
            qDebug() << "删除数据时数据库开启失败";
            return;
        }
    }
    int id=-1;
    if(recordsTable->item(row, 3)->text()=="支出"){
        id = db.getExpenseBillIdByDate(recordsTable->item(row, 1)->text());
    }
    else{
        id = db.getIncomeBillIdByDate(recordsTable->item(row, 1)->text());
    }
    db.deleteRecord(id);

    emit deleteRecordRequested(request);
    emit dataChanged();  // 发出数据变化信号
    loadDayData(currentDate);
}
}

void DayDetailWidget::onBackClicked()
{
emit backToMainView();
}
