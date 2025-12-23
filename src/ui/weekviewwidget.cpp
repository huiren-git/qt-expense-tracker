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
    setStyleSheet("QWidget { background-color: #f5f8fb; }");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 6, 20, 0);

    // 左侧：饼图、排行榜、评论卡片
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(15);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    setupPieChart();
    leftLayout->addWidget(pieChartView);

    setupRankList();
    leftLayout->addWidget(rankListWidget);

    setupCommentCard();
    leftLayout->addWidget(commentLabel);

    mainLayout->addLayout(leftLayout, 1);

    // 右侧：时间选择、卡片、柱状图、周历
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(10);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    // 创建选择器容器
    QWidget *selectorWidget = new QWidget();
    selectorWidget->setStyleSheet("QWidget { background-color: transparent; }");  // 透明背景
    QHBoxLayout *selectorLayout = new QHBoxLayout(selectorWidget);
    selectorLayout->setContentsMargins(0, 5, 0, 5);
    selectorLayout->setSpacing(10);
    selectorWidget->setFixedHeight(45);  // 固定高度

    prevWeekButton = new QPushButton("◀");
    prevWeekButton->setFixedSize(36, 36);
    prevWeekButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 15px; "
        "font-size: 18px; "
        "font-weight: bold; "
        "padding: 0;"
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
        "QPushButton:pressed { background-color: #2d5a8a; }"
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

    nextWeekButton = new QPushButton("▶");
    nextWeekButton->setFixedSize(36, 36);
    nextWeekButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 15px; "
        "font-size: 18px; "
        "font-weight: bold; "
        "padding: 0;"
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
        "QPushButton:pressed { background-color: #2d5a8a; }"
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
    cardLayout->setSpacing(10);
    cardLayout->setContentsMargins(0, 5, 0, 5);
    cardLayout->addWidget(expenseCard);
    cardLayout->addWidget(incomeCard);
    cardLayout->addStretch();
    rightLayout->addLayout(cardLayout);

    setupBarChart();
    rightLayout->addWidget(barChartView,1);

    setupWeekCalendarButtons();
    rightLayout->addLayout(weekButtonsLayout);
    mainLayout->addLayout(rightLayout, 2);

    // 默认选中支出
    switchTransactionType("支出");
}

void WeekViewWidget::setupPieChart()
{
    pieChartView = new QChartView();
    pieChartView->setMinimumSize(300, 300);
    pieChartView->setMaximumSize(300, 300);
    pieChartView->setRenderHint(QPainter::Antialiasing);

    pieSeries = new QPieSeries();
    pieSeries->setHoleSize(0.35);

    // 启用鼠标悬浮放大效果
    connect(pieSeries, &QPieSeries::hovered, this, [this](QPieSlice *slice, bool state) {  // 添加 this 捕获
        if (slice) {
            if (state) {
                // 慢速放大
                slice->setExploded(true);
                slice->setExplodeDistanceFactor(0.05);

                // 显示弹窗
                if (sliceDataMap.contains(slice)) {
                    QJsonObject data = sliceDataMap[slice];
                    QString category = data["category"].toString();
                    double ratio = data["ratio"].toDouble() * 100;
                    double amount = data["totalAmount"].toDouble();

                    QString tooltip = QString("%1\n占比: %2%\n金额: ￥%3")
                        .arg(category)
                        .arg(ratio, 0, 'f', 1)
                        .arg(amount, 0, 'f', 2);

                    QPoint pos = QCursor::pos();
                    QToolTip::showText(pos, tooltip, pieChartView, QRect(), 3000);
                }
            } else {
                slice->setExploded(false);
                QToolTip::hideText();
            }
        }
    });

    QChart *chart = new QChart();
    chart->addSeries(pieSeries);
    chart->setTitle("分类占比");
    chart->legend()->setVisible(false);
    //chart->legend()->setAlignment(Qt::AlignRight);
    chart->setBackgroundBrush(QBrush(QColor("#ffffff")));

    pieChartView->setChart(chart);
}

void WeekViewWidget::setupRankList()
{
    rankListWidget = new QListWidget();
    rankListWidget->setFixedHeight(200);
    rankListWidget->setFixedWidth(300);
    rankListWidget->setSelectionMode(QAbstractItemView::NoSelection);  // 不可选择
    rankListWidget->setStyleSheet(
        "QListWidget { "
        "background-color: white; "
        "border: 1px solid #d0d8e0; "
        "border-radius: 5px; "
        "}"
        "QListWidget::item { "
        "padding: 8px; "
        "border-bottom: 1px solid #e0e8f0; "
        "background-color: transparent; "
        "}"
        "QListWidget::item:hover { "
        "background-color: transparent; "
        "QScrollBar:vertical { "
        "background-color: #f0f4f8; "
        "width: 12px; "
        "border-radius: 6px; "
        "}"
        "QScrollBar::handle:vertical { "
        "background-color: #c0c8d0; "
        "border-radius: 6px; "
        "min-height: 20px; "
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
    expenseCard->setFixedSize(160, 60);
    expenseCard->setCheckable(true);

    QVBoxLayout *expenseLayout = new QVBoxLayout(expenseCard);
    expenseLayout->setContentsMargins(15, 8, 15, 8);
    expenseLayout->setSpacing(0);

    QLabel *expenseLabel1 = new QLabel("支出");
    expenseLabel1->setObjectName("titleLabel");
    QLabel *expenseLabel2 = new QLabel("￥0.00");
    expenseLabel2->setObjectName("amountLabel");
    expenseLabel2->setStyleSheet("font-size: 18px; font-weight: bold; background-color: transparent;");

    expenseLayout->addWidget(expenseLabel1);
    expenseLayout->addWidget(expenseLabel2);
    expenseLayout->addStretch();

    connect(expenseCard, &QPushButton::clicked, this, [this]() {
        switchTransactionType("支出");
    });

    incomeCard = new QPushButton();
    incomeCard->setFixedSize(160, 60);
    incomeCard->setCheckable(true);

    QVBoxLayout *incomeLayout = new QVBoxLayout(incomeCard);
    expenseLayout->setContentsMargins(15, 8, 15, 8);
    incomeLayout->setSpacing(0);

    QLabel *incomeLabel1 = new QLabel("收入");
    incomeLabel1->setObjectName("titleLabel");
    QLabel *incomeLabel2 = new QLabel("￥0.00");
    incomeLabel2->setObjectName("amountLabel");
    incomeLabel2->setStyleSheet("font-size: 18px; font-weight: bold; background-color: transparent;");

    incomeLayout->addWidget(incomeLabel1);
    incomeLayout->addWidget(incomeLabel2);
    incomeLayout->addStretch();

    connect(incomeCard, &QPushButton::clicked, this, [this]() {
        switchTransactionType("收入");
    });
}

void WeekViewWidget::switchTransactionType(const QString &type)
{
    currentTransactionType = type;

    QString activeStyle =
            "QPushButton { "
            "background-color: white; "
            "border: 2px solid #3b6ea5; "
            "border-radius: 10px; "
            "}"
            "QLabel#titleLabel { "
            "color: #3b6ea5; "  // 标题文字变蓝
            "background-color: transparent; "
            "font-size: 12px; "
            "}"
            "QLabel#amountLabel { "
            "color: #3b6ea5; "  // 金额文字变蓝
            "background-color: transparent; "
            "}";

    QString inactiveStyle =
            "QPushButton { "
            "background-color: white; "
            "border: 2px solid #e0e8f0; "
            "border-radius: 10px; "
            "}"
            "QLabel#titleLabel { "
            "color: #999; "  // 未选中时灰色
            "background-color: transparent; "
            "font-size: 12px; "
            "}"
            "QLabel#amountLabel { "
            "color: #999; "  // 未选中时灰色
            "background-color: transparent; "
            "}";

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
    barChartView->setFixedHeight(400);
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

void WeekViewWidget::setupWeekCalendarButtons()
{
    weekButtonsLayout = new QHBoxLayout();
    weekButtonsLayout->setSpacing(5);
    weekButtonsLayout->setContentsMargins(0, 5, 0, 5);

    QStringList weekDays = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};

    for (int i = 0; i < 7; ++i) {
        QPushButton *dayBtn = new QPushButton();
        dayBtn->setCheckable(true);
        dayBtn->setFixedHeight(60); // 增加高度，使内容更清晰
        dayBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        // 样式表：未选中为白底蓝字，选中后为深色背景
        dayBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: white; border: 1px solid #d0d8e0; border-radius: 6px;"
            "  color: #333; font-size: 12px; text-align: center;"
            "}"
            "QPushButton:hover { background-color: #f5f8fb; }"
            "QPushButton:checked { background-color: #3b6ea5; color: white; border: none; }"
        );

        // 绑定点击事件
        connect(dayBtn, &QPushButton::clicked, this, [this, i]() {
            // 互斥逻辑：点击一个，取消其他
            for(auto b : dayButtons) b->setChecked(false);
            dayButtons[i]->setChecked(true);

            // 计算并发送日期
            QDate weekStart = QDate::fromString(QString("%1-W%2-1").arg(currentYear).arg(currentWeek, 2, 10, QChar('0')), "yyyy-'W'ww-d");
            if (weekStart.isValid()) {
                emit dayClicked(weekStart.addDays(i).toString("yyyy-MM-dd"));
            }
        });

        weekButtonsLayout->addWidget(dayBtn);
        dayButtons.append(dayBtn); // 需要在头文件定义 QList<QPushButton*> dayButtons;
    }
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

    // 1. 更新饼图
    QJsonArray pieArray = currentWeekObj["pie"].toArray();
    if (!pieSeries) return;
    pieSeries->clear();
    sliceDataMap.clear();

    for (const QJsonValue &value : pieArray) {
        QJsonObject item = value.toObject();
        QString category = item["category"].toString();
        double amount = item["totalAmount"].toDouble();
        double ratio = item["ratio"].toDouble();
        QPieSlice *slice = pieSeries->append(category, amount);
        slice->setLabelVisible(false);

        QJsonObject sliceData;
        sliceData["category"] = category;
        sliceData["totalAmount"] = amount;
        sliceData["ratio"] = ratio;
        sliceDataMap[slice] = sliceData;
    }

    // 2. 更新排行榜
    rankListWidget->clear();
    int rank = 1;
    for (const QJsonValue &value : pieArray) {
        QJsonObject item = value.toObject();
        QString category = item["category"].toString();
        double amount = item["totalAmount"].toDouble();
        double ratio = item["ratio"].toDouble();
        int count = item["count"].toInt();
        QString text = QString("%1. %2 %3% - ￥%4 (%5笔)")
            .arg(rank++).arg(category).arg(ratio * 100, 0, 'f', 1)
            .arg(amount, 0, 'f', 2).arg(count);
        rankListWidget->addItem(text);
    }

    // 3. 更新评论
    commentLabel->setText(currentWeekObj["comment"].toString());

    // 4. 更新柱状图
    QJsonArray dailyBars = currentWeekObj["dailyBars"].toArray(); // 第一次定义
    QBarSet *currentSet = new QBarSet("本周");
    QBarSet *previousSet = new QBarSet("上周");

    for (int i = 0; i < dailyBars.size() && i < 7; i++) {
        double amount = dailyBars[i].toObject()["dailyAmount"].toDouble();
        *currentSet << amount;
        *previousSet << (amount * 0.8);
    }

    QChart *barChart = barChartView->chart();
    barChart->removeAllSeries();
    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(currentSet);
    barSeries->append(previousSet);
    barChart->addSeries(barSeries);

    // 5. 更新周历按钮
    QDate weekStart = QDate::fromString(QString("%1-W%2-1").arg(currentYear).arg(currentWeek, 2, 10, QChar('0')), "yyyy-'W'ww-d");
    if (!weekStart.isValid()) weekStart = QDate::currentDate();

    QStringList weekDayNames = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};

    // 注意这里：直接使用上面已经定义好的 dailyBars，不要加 QJsonArray
    for (int i = 0; i < 7 && i < dailyBars.size(); i++) {
        QJsonObject dayObj = dailyBars[i].toObject();
        QDate day = weekStart.addDays(i);
        double amount = dayObj["dailyAmount"].toDouble();

        QString btnText = QString("%1\n%2\n￥%3")
                            .arg(weekDayNames[i])
                            .arg(day.toString("MM/dd"))
                            .arg(amount, 0, 'f', 2);

        if (i < dayButtons.size()) {
            dayButtons[i]->setText(btnText);
            // 只有是今天时才高亮显示
            dayButtons[i]->setChecked(day == QDate::currentDate());
        }
    }

    // 6. 更新卡片总金额
    double total = (currentTransactionType == "支出")
        ? currentWeekObj["weeklyExpenseTotal"].toDouble()
        : currentWeekObj["weeklyIncomeTotal"].toDouble();

    QPushButton *activeCard = (currentTransactionType == "支出") ? expenseCard : incomeCard;
    QLabel *amountLabel = activeCard->findChild<QLabel*>("amountLabel");
    if (amountLabel) {
        amountLabel->setText(QString("￥%1").arg(total, 0, 'f', 2));
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
