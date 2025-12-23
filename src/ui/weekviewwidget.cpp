#include "weekviewwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDate>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>

WeekViewWidget::WeekViewWidget(QWidget *parent)
    : QWidget(parent)
    , currentTransactionType("支出")
{
    // 初始化当前周
    QDate today = QDate::currentDate();
    currentYear = today.year();
    currentWeek = today.weekNumber();

    setupUI();
    // 加载测试数据
    loadTestData();
}

void WeekViewWidget::setupUI()
{
    setStyleSheet("QWidget { background-color: #f0f4f8; }");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 左侧：饼图、排行榜、评论卡片
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(15);

    setupPieChart();
    leftLayout->addWidget(pieChartView);

    setupRankList();
    leftLayout->addWidget(rankListWidget);

    setupCommentCard();
    leftLayout->addWidget(commentLabel);

    mainLayout->addLayout(leftLayout, 1);

    // 右侧：时间选择、卡片、柱状图、周历
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(15);

    // 创建选择器容器
    QWidget *selectorWidget = new QWidget();
    QHBoxLayout *selectorLayout = new QHBoxLayout(selectorWidget);
    selectorLayout->setContentsMargins(0, 0, 0, 0);

    prevWeekButton = new QPushButton("←");
    prevWeekButton->setFixedSize(40, 30);
    prevWeekButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
    );
    connect(prevWeekButton, &QPushButton::clicked, this, [this]() {
        if (currentWeek > 1) {
            currentWeek--;
        } else {
            currentYear--;
            currentWeek = 52;
        }
        updateWeekDisplay();
        loadTestData();
    });

    weekRangeLabel = new QLabel();
    weekRangeLabel->setStyleSheet("font-size: 14px; color: #333;");
    updateWeekDisplay();

    nextWeekButton = new QPushButton("→");
    nextWeekButton->setFixedSize(40, 30);
    nextWeekButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
    );
    connect(nextWeekButton, &QPushButton::clicked, this, [this]() {
        if (currentWeek < 52) {
            currentWeek++;
        } else {
            currentYear++;
            currentWeek = 1;
        }
        updateWeekDisplay();
        loadTestData();
    });

    selectorLayout->addWidget(prevWeekButton);
    selectorLayout->addWidget(weekRangeLabel);
    selectorLayout->addWidget(nextWeekButton);
    selectorLayout->addStretch();

    rightLayout->addWidget(selectorWidget);

    setupTransactionTypeCards();
    QHBoxLayout *cardLayout = new QHBoxLayout();
    cardLayout->addWidget(expenseCard);
    cardLayout->addWidget(incomeCard);
    rightLayout->addLayout(cardLayout);

    setupBarChart();
    rightLayout->addWidget(barChartView);

    setupWeekCalendar();
    rightLayout->addWidget(weekCalendarTable);

    mainLayout->addLayout(rightLayout, 2);

    // 默认选中支出
    switchTransactionType("支出");
}

void WeekViewWidget::setupPieChart()
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

void WeekViewWidget::setupRankList()
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
        "QListWidget::item:hover { "
        "background-color: #f0f4f8; "
        "}"
    );
}

void WeekViewWidget::setupCommentCard()
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

void WeekViewWidget::setupTransactionTypeCards()
{
    expenseCard = new QPushButton();
    expenseCard->setFixedSize(150, 100);
    expenseCard->setCheckable(true);

    QVBoxLayout *expenseLayout = new QVBoxLayout(expenseCard);
    expenseLayout->setContentsMargins(10, 10, 10, 10);
    QLabel *expenseLabel1 = new QLabel("支出");
    expenseLabel1->setStyleSheet("font-size: 12px; color: white;");
    QLabel *expenseLabel2 = new QLabel("￥0.00");
    expenseLabel2->setStyleSheet("font-size: 20px; font-weight: bold; color: white;");
    QLabel *expenseLabel3 = new QLabel("元");
    expenseLabel3->setStyleSheet("font-size: 12px; color: white;");
    expenseLayout->addWidget(expenseLabel1);
    expenseLayout->addWidget(expenseLabel2);
    expenseLayout->addWidget(expenseLabel3);

    connect(expenseCard, &QPushButton::clicked, this, [this]() {
        switchTransactionType("支出");
    });

    incomeCard = new QPushButton();
    incomeCard->setFixedSize(150, 100);
    incomeCard->setCheckable(true);

    QVBoxLayout *incomeLayout = new QVBoxLayout(incomeCard);
    incomeLayout->setContentsMargins(10, 10, 10, 10);
    QLabel *incomeLabel1 = new QLabel("收入");
    incomeLabel1->setStyleSheet("font-size: 12px; color: #666;");
    QLabel *incomeLabel2 = new QLabel("￥0.00");
    incomeLabel2->setStyleSheet("font-size: 20px; font-weight: bold; color: #666;");
    QLabel *incomeLabel3 = new QLabel("元");
    incomeLabel3->setStyleSheet("font-size: 12px; color: #666;");
    incomeLayout->addWidget(incomeLabel1);
    incomeLayout->addWidget(incomeLabel2);
    incomeLayout->addWidget(incomeLabel3);

    connect(incomeCard, &QPushButton::clicked, this, [this]() {
        switchTransactionType("收入");
    });
}

void WeekViewWidget::switchTransactionType(const QString &type)
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
        expenseCard->setChecked(true);
        incomeCard->setChecked(false);
    } else {
        expenseCard->setStyleSheet(inactiveStyle);
        incomeCard->setStyleSheet(activeStyle);
        expenseCard->setChecked(false);
        incomeCard->setChecked(true);
    }

    loadTestData();
}

void WeekViewWidget::updateWeekDisplay()
{
    QDate weekStart = QDate::fromString(QString("%1-W%2-1").arg(currentYear).arg(currentWeek, 2, 10, QChar('0')), "yyyy-'W'ww-d");
    if (!weekStart.isValid()) {
        weekStart = QDate::currentDate();
    }
    QDate weekEnd = weekStart.addDays(6);
    weekRangeLabel->setText(QString("%1.%2.%3 - %4.%5.%6")
        .arg(weekStart.year())
        .arg(weekStart.month(), 2, 10, QChar('0'))
        .arg(weekStart.day(), 2, 10, QChar('0'))
        .arg(weekEnd.year())
        .arg(weekEnd.month(), 2, 10, QChar('0'))
        .arg(weekEnd.day(), 2, 10, QChar('0')));
}

void WeekViewWidget::setupBarChart()
{
    barChartView = new QChartView();
    barChartView->setFixedHeight(200);
    barChartView->setRenderHint(QPainter::Antialiasing);

    QBarSet *currentSet = new QBarSet("本周");
    QBarSet *previousSet = new QBarSet("上周");

    *currentSet << 100 << 200 << 150 << 300 << 250 << 180 << 220;
    *previousSet << 80 << 150 << 120 << 250 << 200 << 160 << 180;

    QBarSeries *series = new QBarSeries();
    series->append(currentSet);
    series->append(previousSet);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("一周小结");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setBackgroundBrush(QBrush(QColor("#ffffff")));

    barChartView->setChart(chart);
}

void WeekViewWidget::setupWeekCalendar()
{
    weekCalendarTable = new QTableWidget(1, 7, this);
    weekCalendarTable->setFixedHeight(80);
    weekCalendarTable->horizontalHeader()->setVisible(false);
    weekCalendarTable->verticalHeader()->setVisible(false);
    weekCalendarTable->setShowGrid(true);
    weekCalendarTable->setSelectionMode(QAbstractItemView::SingleSelection);

    weekCalendarTable->setStyleSheet(
        "QTableWidget { "
        "background-color: white; "
        "border: 1px solid #d0d8e0; "
        "gridline-color: #e0e8f0; "
        "}"
        "QTableWidget::item { "
        "padding: 5px; "
        "text-align: center; "
        "}"
        "QTableWidget::item:selected { "
        "background-color: #3b6ea5; "
        "color: white; "
        "}"
    );

    QStringList headers = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};
    for (int i = 0; i < 7; i++) {
        QTableWidgetItem *item = new QTableWidgetItem(headers[i] + "\n￥0.00");
        item->setTextAlignment(Qt::AlignCenter);
        weekCalendarTable->setItem(0, i, item);
    }

    connect(weekCalendarTable, &QTableWidget::cellClicked, this, [this](int row, int col) {
        QDate weekStart = QDate::fromString(QString("%1-W%2-1").arg(currentYear).arg(currentWeek, 2, 10, QChar('0')), "yyyy-'W'ww-d");
        if (!weekStart.isValid()) {
            weekStart = QDate::currentDate();
        }
        QDate clickedDate = weekStart.addDays(col);
        emit dayClicked(clickedDate.toString("yyyy-MM-dd"));
    });
}

void WeekViewWidget::loadWeekData()
{
    // TODO: 调用后端接口获取数据
    // QJsonObject request;
    // request["week"] = currentWeek;
    // request["year"] = currentYear;
    // request["type"] = (currentTransactionType == "支出") ? "expense" : "income";
    // emit queryWeekData(request);
}

void WeekViewWidget::loadTestData()
{
    // 创建测试数据
    QJsonObject testData;
    QJsonObject currentWeekObj;

    currentWeekObj["year"] = currentYear;
    currentWeekObj["week"] = currentWeek;
    currentWeekObj["weeklyIncomeTotal"] = 5200.00;
    currentWeekObj["weeklyExpenseTotal"] = 4100.00;

    // 每日数据
    QJsonArray dailyBars;
    for (int i = 0; i < 7; i++) {
        QJsonObject dayObj;
        QDate weekStart = QDate::fromString(QString("%1-W%2-1").arg(currentYear).arg(currentWeek, 2, 10, QChar('0')), "yyyy-'W'ww-d");
        if (!weekStart.isValid()) {
            weekStart = QDate::currentDate();
        }
        QDate day = weekStart.addDays(i);
        dayObj["date"] = day.toString("yyyy-MM-dd");
        dayObj["hasRecords"] = (i % 2 == 0);
        if (currentTransactionType == "支出") {
            dayObj["dailyAmount"] = (i % 2 == 0) ? (100.0 + i * 50) : 0.0;
            dayObj["dailyExpense"] = dayObj["dailyAmount"];
        } else {
            dayObj["dailyAmount"] = (i % 2 == 0) ? (200.0 + i * 30) : 0.0;
            dayObj["dailyIncome"] = dayObj["dailyAmount"];
        }
        dailyBars.append(dayObj);
    }
    currentWeekObj["dailyBars"] = dailyBars;

    // 饼图数据
    QJsonArray pieArray;
    if (currentTransactionType == "支出") {
        QJsonObject item1;
        item1["category"] = "餐饮美食";
        item1["totalAmount"] = 800.00;
        item1["ratio"] = 0.20;
        item1["count"] = 5;
        pieArray.append(item1);

        QJsonObject item2;
        item2["category"] = "日用百货";
        item2["totalAmount"] = 600.00;
        item2["ratio"] = 0.15;
        item2["count"] = 3;
        pieArray.append(item2);

        QJsonObject item3;
        item3["category"] = "服饰装扮";
        item3["totalAmount"] = 500.00;
        item3["ratio"] = 0.12;
        item3["count"] = 2;
        pieArray.append(item3);
    } else {
        QJsonObject item1;
        item1["category"] = "收入";
        item1["totalAmount"] = 5200.00;
        item1["ratio"] = 1.00;
        item1["count"] = 2;
        pieArray.append(item1);
    }
    currentWeekObj["pie"] = pieArray;
    currentWeekObj["comment"] = "实用才是第一原则！";

    testData["operation"] = true;
    testData["currentWeek"] = currentWeekObj;

    updateWeekData(testData);
}

void WeekViewWidget::updateWeekData(const QJsonObject &data)
{
    QJsonObject currentWeekObj = data["currentWeek"].toObject();

    // 更新饼图
    QJsonArray pieArray = currentWeekObj["pie"].toArray();
    QChart *chart = pieChartView->chart();
    QPieSeries *series = qobject_cast<QPieSeries*>(chart->series().first());
    if (series) {
        series->clear();

        for (const QJsonValue &value : pieArray) {
            QJsonObject item = value.toObject();
            QString category = item["category"].toString();
            double amount = item["totalAmount"].toDouble();
            series->append(category, amount);
        }
    }

    // 更新排行榜
    rankListWidget->clear();
    int rank = 1;
    for (const QJsonValue &value : pieArray) {
        QJsonObject item = value.toObject();
        QString category = item["category"].toString();
        double amount = item["totalAmount"].toDouble();
        double ratio = item["ratio"].toDouble();
        int count = item["count"].toInt();

        QString text = QString("%1. %2 %3% - ￥%4 (%5笔)")
            .arg(rank++)
            .arg(category)
            .arg(ratio * 100, 0, 'f', 1)
            .arg(amount, 0, 'f', 2)
            .arg(count);
        rankListWidget->addItem(text);
    }

    // 更新评论
    QString comment = currentWeekObj["comment"].toString();
    commentLabel->setText(comment);

    // 更新柱状图
    QJsonArray dailyBars = currentWeekObj["dailyBars"].toArray();
    QBarSet *currentSet = new QBarSet("本周");
    QBarSet *previousSet = new QBarSet("上周");

    for (int i = 0; i < dailyBars.size() && i < 7; i++) {
        QJsonObject dayObj = dailyBars[i].toObject();
        double amount = dayObj["dailyAmount"].toDouble();
        *currentSet << amount;
        *previousSet << (amount * 0.8); // 模拟上周数据
    }

    QChart *barChart = barChartView->chart();
    barChart->removeAllSeries();
    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(currentSet);
    barSeries->append(previousSet);
    barChart->addSeries(barSeries);

    // 更新周历
    QDate weekStart = QDate::fromString(QString("%1-W%2-1").arg(currentYear).arg(currentWeek, 2, 10, QChar('0')), "yyyy-'W'ww-d");
    if (!weekStart.isValid()) {
        weekStart = QDate::currentDate();
    }
    QDate today = QDate::currentDate();

    for (int i = 0; i < 7 && i < dailyBars.size(); i++) {
        QJsonObject dayObj = dailyBars[i].toObject();
        QDate day = weekStart.addDays(i);
        double amount = dayObj["dailyAmount"].toDouble();

        QString dayText = QString("%1\n￥%2")
            .arg(day.toString("MM/dd"))
            .arg(amount, 0, 'f', 2);

        QTableWidgetItem *item = weekCalendarTable->item(0, i);
        if (item) {
            item->setText(dayText);
            if (day == today) {
                item->setBackground(QColor("#3b6ea5"));
                item->setForeground(QColor("white"));
            } else {
                item->setBackground(QColor("white"));
                item->setForeground(QColor("black"));
            }
        }
    }

    // 更新卡片金额
    double total = (currentTransactionType == "支出")
        ? currentWeekObj["weeklyExpenseTotal"].toDouble()
        : currentWeekObj["weeklyIncomeTotal"].toDouble();

    QVBoxLayout *cardLayout = qobject_cast<QVBoxLayout*>(
        (currentTransactionType == "支出") ? expenseCard->layout() : incomeCard->layout()
    );
    if (cardLayout) {
        QLabel *amountLabel = qobject_cast<QLabel*>(cardLayout->itemAt(1)->widget());
        if (amountLabel) {
            amountLabel->setText(QString("￥%1").arg(total, 0, 'f', 2));
        }
    }
}

void WeekViewWidget::onDayClicked(const QString &date)
{
    // 暂时为空实现，仅用于测试界面
    // TODO: 后续实现点击日期后的逻辑
    Q_UNUSED(date);
}

void WeekViewWidget::onWeekChanged(int year, int week)
{
    currentYear = year;
    currentWeek = week;
    updateWeekDisplay();
    loadTestData();  // 使用测试数据
}
