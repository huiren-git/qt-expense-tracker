#include "recordeditdialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QWidget>
#include <QLabel>
#include <QJsonObject>
#include <QIntValidator>
#include <QDoubleValidator>


RecordEditDialog::RecordEditDialog(QWidget *parent, bool isEdit)
    : QDialog(parent)
    , isEditMode(isEdit)
{
    setWindowTitle(isEdit ? "编辑记录" : "插入新记录");
    setFixedSize(500, 600);
    setStyleSheet("QDialog { background-color: #f0f4f8; }");

    setupUI();
}

RecordEditDialog::~RecordEditDialog()
{
}

void RecordEditDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    setupForm();
    mainLayout->addLayout(formLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    confirmButton = new QPushButton("确定");
    confirmButton->setFixedSize(80, 35);
    confirmButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
    );
    connect(confirmButton, &QPushButton::clicked, this, &RecordEditDialog::onConfirmClicked);

    cancelButton = new QPushButton("取消");
    cancelButton->setFixedSize(80, 35);
    cancelButton->setStyleSheet(
        "QPushButton { "
        "background-color: #e0e8f0; "
        "color: #333; "
        "border: none; "
        "border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #d0d8e0; }"
    );
    connect(cancelButton, &QPushButton::clicked, this, &RecordEditDialog::onCancelClicked);

    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
}

void RecordEditDialog::setupForm()
{
    formLayout = new QFormLayout();
    formLayout->setSpacing(10);
    formLayout->setLabelAlignment(Qt::AlignRight);

    setupDateTimeInputs();
    formLayout->addRow("交易时间:", dateTimeWidget);

    setupAmountEdit();
    formLayout->addRow("交易金额:", amountEdit);

    setupTransactionTypeRadio();
    formLayout->addRow("交易类型:", transactionTypeWidget);

    setupCategoryComboBox();
    formLayout->addRow("交易分类:", categoryComboBox);

    setupTransactionMethodRadio();
    formLayout->addRow("交易方式:", transactionMethodWidget);

    counterpartyEdit = new QLineEdit();
    formLayout->addRow("交易对方:", counterpartyEdit);

    productNameEdit = new QLineEdit();
    formLayout->addRow("交易内容:", productNameEdit);

    remarkEdit = new QLineEdit();
    formLayout->addRow("备注:", remarkEdit);

    sourceIdEdit = new QLineEdit();
    formLayout->addRow("订单号:", sourceIdEdit);
}

void RecordEditDialog::setupDateTimeInputs()
{
    dateTimeWidget = new QWidget();
    QHBoxLayout *dateTimeLayout = new QHBoxLayout(dateTimeWidget);
    dateTimeLayout->setContentsMargins(0, 0, 0, 0);
    dateTimeLayout->setSpacing(5);

    yearEdit = new QLineEdit();
    yearEdit->setFixedWidth(60);
    yearEdit->setPlaceholderText("年");
    QRegExp regYearExp("^([2][0][0-9][0-9])$");
    QValidator *regYearValidator = new QRegExpValidator(regYearExp);
    yearEdit->setValidator(regYearValidator);

    monthEdit = new QLineEdit();
    monthEdit->setFixedWidth(40);
    monthEdit->setPlaceholderText("月");
    QRegExp regMonthExp("^([1-9]|[1][0-2])$");
    QValidator *regMonthValidator = new QRegExpValidator(regMonthExp);
    monthEdit->setValidator(regMonthValidator);

    dayEdit = new QLineEdit();
    dayEdit->setFixedWidth(40);
    dayEdit->setPlaceholderText("日");
    QRegExp regDayExp("^([1-9]|[1-2][0-9]|[3][0-1])$");
    QValidator *regDayValidator = new QRegExpValidator(regDayExp);
    dayEdit->setValidator(regDayValidator);

    hourEdit = new QLineEdit();
    hourEdit->setFixedWidth(40);
    hourEdit->setPlaceholderText("时");
    QRegExp regHourExp("^([0-9]|[1][0-9]|[2][0-3])$");
    QValidator *regHourValidator = new QRegExpValidator(regHourExp);
    hourEdit->setValidator(regHourValidator);

    minuteEdit = new QLineEdit();
    minuteEdit->setFixedWidth(40);
    minuteEdit->setPlaceholderText("分");
    QRegExp regMinuteExp("^([0-9]|[1-5][0-9])$");
    QValidator *regMinuteValidator = new QRegExpValidator(regMinuteExp);
    minuteEdit->setValidator(regMinuteValidator);

    secondEdit = new QLineEdit();
    secondEdit->setFixedWidth(40);
    secondEdit->setPlaceholderText("秒");
    //因为秒和分限制相同使用同一验证器
    secondEdit->setValidator(regMinuteValidator);

    dateTimeLayout->addWidget(yearEdit);
    dateTimeLayout->addWidget(new QLabel("-"));
    dateTimeLayout->addWidget(monthEdit);
    dateTimeLayout->addWidget(new QLabel("-"));
    dateTimeLayout->addWidget(dayEdit);
    dateTimeLayout->addWidget(new QLabel(" "));
    dateTimeLayout->addWidget(hourEdit);
    dateTimeLayout->addWidget(new QLabel(":"));
    dateTimeLayout->addWidget(minuteEdit);
    dateTimeLayout->addWidget(new QLabel(":"));
    dateTimeLayout->addWidget(secondEdit);
    dateTimeLayout->addStretch();

    // 默认填充当前时间
    QDateTime now = QDateTime::currentDateTime();
    yearEdit->setText(QString::number(now.date().year()));
    monthEdit->setText(QString::number(now.date().month()));
    dayEdit->setText(QString::number(now.date().day()));
    hourEdit->setText(QString::number(now.time().hour()));
    minuteEdit->setText(QString::number(now.time().minute()));
    secondEdit->setText(QString::number(now.time().second()));
}

void RecordEditDialog::setupAmountEdit(){
    //创建控件
    amountEdit = new QLineEdit();

    //设置金额范围
    amountEdit->setPlaceholderText("元");
    QDoubleValidator *validator = new QDoubleValidator(0,999999999,2,this);
    validator->setNotation(QDoubleValidator::StandardNotation);
    amountEdit->setValidator(validator);
}

void RecordEditDialog::setupTransactionTypeRadio()
{
    transactionTypeWidget = new QWidget();
    QHBoxLayout *typeLayout = new QHBoxLayout(transactionTypeWidget);
    typeLayout->setContentsMargins(0, 0, 0, 0);

    transactionTypeGroup = new QButtonGroup(this);

    expenseRadio = new QRadioButton("支出");
    expenseRadio->setChecked(true);
    transactionTypeGroup->addButton(expenseRadio, 0);

    incomeRadio = new QRadioButton("收入");
    transactionTypeGroup->addButton(incomeRadio, 1);

    typeLayout->addWidget(expenseRadio);
    typeLayout->addWidget(incomeRadio);
    typeLayout->addStretch();
}

void RecordEditDialog::setupCategoryComboBox()
{
    categoryComboBox = new QComboBox();
    categoryComboBox->addItems({
        "餐饮美食", "服饰装扮", "日用百货", "家居家装", "数码电器",
        "运动户外", "美容美发", "母婴亲子", "宠物", "交通出行",
        "爱车养车", "住房物业", "酒店旅游", "文化休闲", "教育培训",
        "医疗健康", "生活服务", "公共服务", "商业服务", "公益捐赠",
        "互助保障", "投资理财", "保险", "信用借还", "充值缴费",
        "收入", "转账红包", "亲友代付", "账户存取", "退款", "其他"
    });
}

void RecordEditDialog::setupTransactionMethodRadio()
{
    transactionMethodWidget = new QWidget();
    QHBoxLayout *methodLayout = new QHBoxLayout(transactionMethodWidget);
    methodLayout->setContentsMargins(0, 0, 0, 0);

    transactionMethodGroup = new QButtonGroup(this);

    alipayRadio = new QRadioButton("支付宝");
    alipayRadio->setChecked(true);
    transactionMethodGroup->addButton(alipayRadio, 0);

    cashRadio = new QRadioButton("现金");
    transactionMethodGroup->addButton(cashRadio, 1);

    othersRadio = new QRadioButton("其他");
    transactionMethodGroup->addButton(othersRadio, 2);

    methodLayout->addWidget(alipayRadio);
    methodLayout->addWidget(cashRadio);
    methodLayout->addWidget(othersRadio);
    methodLayout->addStretch();
}

QJsonObject RecordEditDialog::getRecordData() const
{
    QJsonObject record;

    QString dateTimeStr = QString("%1-%2-%3 %4:%5:%6")
        .arg(yearEdit->text(), 4, QChar('0'))
        .arg(monthEdit->text(), 2, QChar('0'))
        .arg(dayEdit->text(), 2, QChar('0'))
        .arg(hourEdit->text(), 2, QChar('0'))
        .arg(minuteEdit->text(), 2, QChar('0'))
        .arg(secondEdit->text(), 2, QChar('0'));

    record["transactionDate"] = dateTimeStr;

    QDate date(yearEdit->text().toInt(), monthEdit->text().toInt(), dayEdit->text().toInt());
    record["year"] = date.year();
    record["month"] = date.month();
    record["week"] = date.weekNumber();

    record["amount"] = amountEdit->text().toDouble();
    record["transactionType"] = expenseRadio->isChecked() ? "income" : "expense";
    record["category"] = categoryComboBox->currentIndex()+1;

    if (alipayRadio->isChecked()) {
        record["transactionMethod"] = 1;
    } else if (cashRadio->isChecked()) {
        record["transactionMethod"] = 2;
    } else {
        record["transactionMethod"] = 3;
    }

    record["counterparty"] = counterpartyEdit->text();
    record["productName"] = productNameEdit->text();
    record["remark"] = remarkEdit->text();
    record["sourceId"] = sourceIdEdit->text().isEmpty() ? QJsonValue() : sourceIdEdit->text();

    return record;
}

void RecordEditDialog::setRecordData(const QJsonObject &record)
{
    QString dateTimeStr = record["transactionDate"].toString();
    QStringList parts = dateTimeStr.split(" ");
    QStringList dateParts = parts[0].split("-");
    QStringList timeParts = parts[1].split(":");

    if (dateParts.size() == 3) {
        yearEdit->setText(dateParts[0]);
        monthEdit->setText(dateParts[1]);
        dayEdit->setText(dateParts[2]);
    }

    if (timeParts.size() == 3) {
        hourEdit->setText(timeParts[0]);
        minuteEdit->setText(timeParts[1]);
        secondEdit->setText(timeParts[2]);
    }

    amountEdit->setText(record["amount"].toString());

    QString transactionType = record["transactionType"].toString();
    if (transactionType == "支出") {
        expenseRadio->setChecked(true);
    } else {
        incomeRadio->setChecked(true);
    }

    QString category = record["category"].toString();
    int index = categoryComboBox->findText(category);
    if (index >= 0) {
        categoryComboBox->setCurrentIndex(index);
    }

    QString method = record["transactionMethod"].toString();
    if (method == "支付宝") {
        alipayRadio->setChecked(true);
    } else if (method == "现金") {
        cashRadio->setChecked(true);
    } else {
        othersRadio->setChecked(true);
    }

    counterpartyEdit->setText(record["counterparty"].toString());
    productNameEdit->setText(record["productName"].toString());
    remarkEdit->setText(record["remark"].toString());
    sourceIdEdit->setText(record["sourceId"].toString());
}

void RecordEditDialog::onConfirmClicked()
{
    accept();
}

void RecordEditDialog::onCancelClicked()
{
    reject();
}
