#include "monthviewwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDate>
#include <QJsonObject>
#include <QJsonArray>

MonthViewWidget::MonthViewWidget(QWidget *parent)
    : QWidget(parent)
    , currentTransactionType("支出")
{
    QDate today = QDate::currentDate();
    currentYear = today.year();
    currentMonth = today.month();

    setupUI();
}

void MonthViewWidget::setupUI()
{
    setStyleSheet("QWidget { background-color: #f0f4f8; }");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 左侧
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(15);

    setupPieChart();
    leftLayout->addWidget(pieChartView);

    setupRankList();
    leftLayout->addWidget(rankListWidget);

    setupCommentCard();
    leftLayout->addWidget(commentLabel);

    mainLayout->addLayout(leftLayout, 1);

    // 右侧
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(15);

    setupMonthSelector();
    rightLayout->addLayout(monthSelectorLayout);

    setupTransactionTypeCards();
    QHBoxLayout *cardLayout = new QHBoxLayout();
    cardLayout->addWidget(expenseCard);
    cardLayout->addWidget(incomeCard);
    rightLayout->addLayout(cardLayout);

    setupCalendar();
    rightLayout->addWidget(calendarWidget);

    mainLayout->addLayout(rightLayout, 2);

    switchTransactionType("支出");
}

void MonthViewWidget::setupPieChart()
{
    pieChartView = new QChartView();
    pieChartView->setFixedSize(300, 300);
    pieChartView->setRenderHint(QPainter::Antialiasing);

    QPieSeries *series = new QPieSeries();
    series->setHoleSize(0.35);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("分类占比");
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->setBackgroundBrush(QBrush(QColor("#ffffff")));

    pieChartView->setChart(chart);
}

void MonthViewWidget::setupRankList()
{
    rankListWidget = new QListWidget();
    rankListWidget->setFixedHeight(200);
    rankListWidget->setStyleSheet(
        "QListWidget { "
        "background-color: white; "
        "border: 1px solid #d0d8e0; "
        "border-radius: 5px; "
        "}"
        "QListWidget::item { "
        "padding: 8px; "
        "border-bottom: 1px solid #e0e8f0; "
        "}"
    );
}

void MonthViewWidget::setupCommentCard()
{
    commentLabel = new QLabel("实用才是第一原则！");
    commentLabel->setFixedHeight(80);
    commentLabel->setStyleSheet(
        "QLabel { "
        "background-color: white; "
        "border: 2px dashed #3b6ea5; "
        "border-radius: 10px; "
        "padding: 15px; "
        "font-size: 14px; "
        "color: #333; "
        "}"
    );
    commentLabel->setWordWrap(true);
    commentLabel->setAlignment(Qt::AlignCenter);
}

void MonthViewWidget::setupMonthSelector()
{
    monthSelectorLayout = new QHBoxLayout();

    yearComboBox = new QComboBox();
    yearComboBox->setFixedWidth(100);
    for (int y = 2020; y <= 2030; y++) {
        yearComboBox->addItem(QString::number(y));
    }
    yearComboBox->setCurrentText(QString::number(currentYear));
    connect(yearComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        onMonthChanged(yearComboBox->currentText().toInt(), monthComboBox->currentText().toInt());
    });

    monthComboBox = new QComboBox();
    monthComboBox->setFixedWidth(80);
    for (int m = 1; m <= 12; m++) {
        monthComboBox->addItem(QString::number(m));
    }
    monthComboBox->setCurrentText(QString::number(currentMonth));
    connect(monthComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        onMonthChanged(yearComboBox->currentText().toInt(), monthComboBox->currentText().toInt());
    });

    monthSelectorLayout->addWidget(yearComboBox);
    monthSelectorLayout->addWidget(new QLabel("年"));
    monthSelectorLayout->addWidget(monthComboBox);
    monthSelectorLayout->addWidget(new QLabel("月"));
    monthSelectorLayout->addStretch();
}

void MonthViewWidget::setupTransactionTypeCards()
{
    expenseCard = new QPushButton();
    expenseCard->setFixedSize(150, 100);
    expenseCard->setCheckable(true);
    connect(expenseCard, &QPushButton::clicked, this, [this]() {
        switchTransactionType("支出");
    });

    incomeCard = new QPushButton();
    incomeCard->setFixedSize(150, 100);
    incomeCard->setCheckable(true);
    connect(incomeCard, &QPushButton::clicked, this, [this]() {
        switchTransactionType("收入");
    });
}

void MonthViewWidget::switchTransactionType(const QString &type)
{
    currentTransactionType = type;

    QString activeStyle =
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 10px; "
        "text-align: left; "
        "padding: 10px; "
        "}"
        "QLabel { color: white; }";

    QString inactiveStyle =
        "QPushButton { "
        "background-color: #e0e8f0; "
        "color: #666; "
        "border: none; "
        "border-radius: 10px; "
        "text-align: left; "
        "padding: 10px; "
        "}"
        "QLabel { color: #666; }";

    if (type == "支出") {
        expenseCard->setStyleSheet(activeStyle);
        incomeCard->setStyleSheet(inactiveStyle);
    } else {
        expenseCard->setStyleSheet(inactiveStyle);
        incomeCard->setStyleSheet(activeStyle);
    }

    loadMonthData();
}

void MonthViewWidget::setupCalendar()
{
    calendarWidget = new QCalendarWidget();
    calendarWidget->setFixedSize(600, 400);
    calendarWidget->setStyleSheet(
        "QCalendarWidget { "
        "background-color: white; "
        "border: 1px solid #d0d8e0; "
        "border-radius: 5px; "
        "}"
        "QCalendarWidget QTableView { "
        "selection-background-color: #3b6ea5; "
        "}"
    );

    connect(calendarWidget, &QCalendarWidget::clicked, this, [this](const QDate &date) {
        onDayClicked(date.toString("yyyy-MM-dd"));
    });
}

void MonthViewWidget::onMonthChanged(int year, int month)
{
    currentYear = year;
    currentMonth = month;
    loadMonthData();
}

void MonthViewWidget::onDayClicked(const QString &date)
{
    emit dayClicked(date);
}

void MonthViewWidget::loadMonthData()
{
    // TODO: 调用后端接口
}
