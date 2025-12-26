#include "weekviewwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDate>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QFont>
#include <QSqlQuery>
#include <QDebug>
#include "../db/database_manager.h"


WeekViewWidget::WeekViewWidget(QWidget *parent)
    : QWidget(parent)
    , currentTransactionType("支出")
{
    // 初始化当前周
    QDate today = QDate::currentDate();
    currentYear = today.year();
    currentWeek = today.weekNumber();
    QFont dengXianFont("DengXian");
    dengXianFont.setPixelSize(12); // 设置字号

    setupUI();
    // 加载数据
    loadWeekData();
}

void WeekViewWidget::setupUI()
{

    setStyleSheet("QWidget { background-color: #f5f8fb; }");
    this->setFont(QFont("DengXian", 12));
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
        loadWeekData();
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
        loadWeekData();
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
    connect(pieSeries, &QPieSeries::hovered, this, [this](QPieSlice *slice, bool state) {
            if (!slice) return;

            if (state) {
                // 1. 视觉增强：突出显示
                slice->setExploded(true);
                slice->setExplodeDistanceFactor(0.05); // 稍微弹出的距离
                slice->setPen(QPen(QColor("#3b6ea5"), 2)); // 增加边框颜色

                // 2. 显示精美浮窗 (ToolTip)
                if (sliceDataMap.contains(slice)) {
                    QJsonObject data = sliceDataMap[slice];
                    QString category = data["category"].toString();
                    double ratio = data["ratio"].toDouble() * 100;
                    double amount = data["totalAmount"].toDouble();

                    // 使用 HTML 格式美化 ToolTip
                    QString tooltip = QString(
                        "<div style='font-family: Microsoft YaHei;'>"
                        "<b>类别:</b> %1<br/>"
                        "<b>占比:</b> <span style='color:#3b6ea5;'>%2%</span><br/>"
                        "<b>金额:</b> ￥%3"
                        "</div>"
                    ).arg(category).arg(ratio, 0, 'f', 1).arg(amount, 0, 'f', 2);

                    // 在鼠标当前位置显示
                    QToolTip::showText(QCursor::pos(), tooltip, pieChartView);
                }
            } else {
                // 还原状态
                slice->setExploded(false);
                slice->setPen(QPen(Qt::NoPen)); // 移除边框
                QToolTip::hideText();
            }
        });

        // 3. 启用动画（关键：实现“缓慢”感）
        QChart *chart = new QChart();
        chart->addSeries(pieSeries);
        chart->setTitle("分类占比");

        // 设置动画效果为全部启用
        chart->setAnimationOptions(QChart::AllAnimations);
        // 注意：Qt Charts 的动画速度受全局配置影响，AllAnimations 会让 Explode 动作更柔和

        chart->legend()->setVisible(false);
        chart->setBackgroundBrush(QBrush(QColor("#ffffff")));
        pieChartView->setChart(chart);
}

void WeekViewWidget::setupRankList()
{
    rankListWidget = new QListWidget();
    rankListWidget->setFixedHeight(200);
    rankListWidget->setFixedWidth(300);
    rankListWidget->setSelectionMode(QAbstractItemView::NoSelection); 
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

    loadWeekData();
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

    connect(series, &QBarSeries::hovered, this, [this](bool status, int index, QBarSet *barset) {
            if (status) {
                // 1. 视觉反馈：加粗边框或改变透明度
                barset->setPen(QPen(QColor("#1e3a5f"), 2));

                // 2. 获取数据并显示弹窗
                double value = barset->at(index);
                QString label = barset->label(); // "本周" 或 "上周"
                QStringList weekDayNames = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};
                QString dayName = (index < 7) ? weekDayNames[index] : "";

                QString tooltip = QString(
                    "<div style='font-family: Microsoft YaHei;'>"
                    "<b>%1</b> (%2)<br/>"
                    "金额: <span style='color:#3b6ea5; font-size:14px;'>￥%3</span>"
                    "</div>"
                ).arg(dayName).arg(label).arg(value, 0, 'f', 2);

                QToolTip::showText(QCursor::pos(), tooltip, barChartView);
            } else {
                // 还原状态：清除边框
                barset->setPen(QPen(Qt::NoPen));
                QToolTip::hideText();
            }
        });

        // 必须开启全动画，才能让边框变化显得平滑
        chart->setAnimationOptions(QChart::AllAnimations);

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
        dayBtn->setFixedHeight(60);
        dayBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        dayBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: white; border: 1px solid #d0d8e0; border-radius: 6px;"
            "  color: #333; font-size: 12px; text-align: center;"
            "}"
            "QPushButton:hover { background-color: #f5f8fb; }"
            "QPushButton:checked { background-color: #3b6ea5; color: white; border: none; }"
        );

        // 绑定点击事件 - 修复：使用正确的 ISO 周计算方式
        connect(dayBtn, &QPushButton::clicked, this, [this, i]() {
            // 互斥逻辑：点击一个，取消其他
            for(auto b : dayButtons) b->setChecked(false);
            dayButtons[i]->setChecked(true);

            // 计算并发送日期 - 使用 getMondayOfISOWeek 函数
            QDate weekStart = getMondayOfISOWeek(currentYear, currentWeek);
            QString dateStr = weekStart.addDays(i).toString("yyyy-MM-dd");
            emit dayClicked(dateStr);
        });

        weekButtonsLayout->addWidget(dayBtn);
        dayButtons.append(dayBtn);
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

    //初始配置，打开数据库，初始化时间
    DatabaseManager &db = DatabaseManager::instance();
    if(!db.isReady()){
        if(!db.openDatabase()){
            loadTestData();
            return;
        }
    }
    QJsonObject response;
    response["operation"] = true;
    QJsonObject currentWeekObj;
    currentWeekObj["year"] = currentYear;
    currentWeekObj["week"] = currentWeek;
    // 周总收支
    QSqlQuery query = db.getTotalIncomeByWeek(currentYear,currentWeek);
    query.next();
    double income=query.value(0).toDouble();
    currentWeekObj["weeklyIncomeTotal"] = income;

    query = db.getTotalExpenseByWeek(currentYear,currentWeek);
    query.next();
    double expense=query.value(0).toDouble();
    qDebug() << expense;
    currentWeekObj["weeklyExpenseTotal"] = expense;

    //单日收支
    QJsonArray currentBars;
    QJsonArray previousBars;
    QDate weekStart=getMondayOfISOWeek(currentYear, currentWeek);
    for (int i = 0; i < 7; i++) {
        query=db.getTotalRecordsByDay(weekStart.addDays(i).toString("yyyy-MM-dd"));
        query.next();
        QJsonObject cDay;
        cDay["dailyExpense"] = query.value(0).toDouble();
        cDay["dailyIncome"] = query.value(1).toDouble();
        currentBars.append(cDay);

        query=db.getTotalRecordsByDay(weekStart.addDays(i-7).toString("yyyy-MM-dd"));
        query.next();
        QJsonObject pDay;
        pDay["dailyExpense"] = query.value(0).toDouble();
        pDay["dailyIncome"] = query.value(1).toDouble();
        previousBars.append(pDay);
    }
    currentWeekObj["dailyBars"] = currentBars;

    QJsonObject previousWeekObj;
    previousWeekObj["dailyBars"] = previousBars;

    // 饼图数据根据类型变化
    QJsonArray pieArray;
    QJsonObject cat;
    if (currentTransactionType == "支出") {
        query = db.getExpenseCategoryStatsByWeek(currentYear,currentWeek);
        while(query.next()){
            cat["category"] = query.value(0).toString();
            cat["totalAmount"] = query.value(2).toDouble();
            cat["ratio"] = query.value(2).toDouble()/expense;
            cat["count"] = query.value(1).toInt();
            pieArray.append(cat);
        }

    } else {
        query = db.getIncomeCategoryStatsByWeek(currentYear,currentWeek);
        while(query.next()){
            cat["category"] = query.value(0).toString();
            cat["totalAmount"] = query.value(2).toDouble();
            cat["ratio"] = query.value(2).toDouble()/income;
            cat["count"] = query.value(1).toInt();
            pieArray.append(cat);
        }
    }
    currentWeekObj["pie"] = pieArray;
    QString type;
    if(currentTransactionType == "支出"){
        type = "expense";
    }
    else{
        type = "income";
    }
    QString comment = db.getTopCategoryByWeekWithComment(currentYear,currentWeek,type);
    //currentWeekObj["comment"] = (currentTransactionType == "支出") ? "节约是美德" : "加油赚钱！";
    currentWeekObj["comment"] = comment;
    response["currentWeek"] = currentWeekObj;
    response["previousWeek"] = previousWeekObj;

    updateWeekData(response);

}

void WeekViewWidget::loadTestData()
{
    QJsonObject response;
    response["operation"] = true;
    QJsonObject currentWeekObj;
    currentWeekObj["year"] = currentYear;
    currentWeekObj["week"] = currentWeek;
    DatabaseManager &db = DatabaseManager::instance();
    if(!db.isReady()){
        if(!db.openDatabase()){
            //return;
        }
    }
    // 模拟数据
    currentWeekObj["weeklyIncomeTotal"] = 5000.0 + (currentWeek * 10);
    currentWeekObj["weeklyExpenseTotal"] = 3000.0 + (currentWeek * 5);

    QJsonArray currentBars;
    QJsonArray previousBars;
    for (int i = 0; i < 7; i++) {
        QJsonObject cDay;
        cDay["dailyExpense"] = 100.0 + (i * 20);
        cDay["dailyIncome"] = 200.0 + (i * 10);
        currentBars.append(cDay);

        QJsonObject pDay;
        pDay["dailyExpense"] = 90.0 + (i * 15);
        pDay["dailyIncome"] = 180.0 + (i * 5);
        previousBars.append(pDay);
    }
    currentWeekObj["dailyBars"] = currentBars;

    QJsonObject previousWeekObj;
    previousWeekObj["dailyBars"] = previousBars;

    // 饼图数据根据类型变化
    QJsonArray pieArray;
    QJsonObject cat;
    if (currentTransactionType == "支出") {
        cat["category"] = "餐饮美食";
        cat["totalAmount"] = currentWeekObj["weeklyExpenseTotal"];
    } else {
        cat["category"] = "薪资收入";
        cat["totalAmount"] = currentWeekObj["weeklyIncomeTotal"];
    }
    cat["ratio"] = 1.0;
    pieArray.append(cat);
    currentWeekObj["pie"] = pieArray;
    currentWeekObj["comment"] = (currentTransactionType == "支出") ? "节约是美德" : "加油赚钱！";

    response["currentWeek"] = currentWeekObj;
    response["previousWeek"] = previousWeekObj;

    updateWeekData(response);
}

QDate WeekViewWidget::getMondayOfISOWeek(int year, int week)
{
    // 1月4日总是落在 ISO 周的第一周
    QDate day(year, 1, 4);
    // 找到该周的周一
    int daysToMonday = day.dayOfWeek() - 1;
    QDate monday1 = day.addDays(-daysToMonday);
    // 加上周数差
    return monday1.addDays((week - 1) * 7);
}


void WeekViewWidget::updateWeekData(const QJsonObject &json)
{
    // 1. 协议基础检查
    if (!json["operation"].toBool()) return;
    QJsonObject current = json["currentWeek"].toObject();
    QJsonObject previous = json["previousWeek"].toObject();

    // --- A. 更新顶部日期和周数文字 ---
    int year = current["year"].toInt();
    int week = current["week"].toInt();
    // 使用 ISO 算法计算日期范围，确保中间文字必变
    QDate firstDay(year, 1, 4);
    QDate weekStart = firstDay.addDays(-(firstDay.dayOfWeek() - 1)).addDays((week - 1) * 7);
    QDate weekEnd = weekStart.addDays(6);
    weekRangeLabel->setText(QString("%1 - %2")
        .arg(weekStart.toString("yyyy.MM.dd")).arg(weekEnd.toString("yyyy.MM.dd")));
    weekRangeLabel->setFont(QFont("DengXian", 12, QFont::Bold));

    // --- B. 更新饼图与排行榜 (协议字段: "pie") ---
    QJsonArray pieArray = current["pie"].toArray();
    if (pieSeries) {
        pieSeries->clear();
        sliceDataMap.clear();
        rankListWidget->clear(); // 清空旧排行榜
        int rank = 1;

        for (const QJsonValue &value : pieArray) {
            QJsonObject item = value.toObject();
            QString category = item["category"].toString();
            double amount = item["totalAmount"].toDouble();

            // 更新饼图 Series
            QPieSlice *slice = pieSeries->append(category, amount);
            slice->setLabelVisible(false);
            sliceDataMap[slice] = item; // 存储原始 JSON 对象供 Hover 使用

            // 更新左侧排行榜 (使用等线字体)
            QString rankText = QString("%1. %2 %3% - ￥%4")
                .arg(rank++).arg(category)
                .arg(item["ratio"].toDouble() * 100, 0, 'f', 1)
                .arg(amount, 0, 'f', 2);
            QListWidgetItem *rankItem = new QListWidgetItem(rankText);
            rankItem->setFont(QFont("DengXian", 10));
            rankListWidget->addItem(rankItem);
        }
    }

    // --- C. 更新柱状图 (协议字段: "dailyBars") ---
    QJsonArray currentBars = current["dailyBars"].toArray();
        QJsonArray prevBars = previous["dailyBars"].toArray();

        QChart *barChart = barChartView->chart();
        barChart->setAnimationOptions(QChart::NoAnimation);
        barChart->removeAllSeries();

        QBarSeries *newBarSeries = new QBarSeries();
        QBarSet *setThisWeek = new QBarSet("本周" + currentTransactionType);
        QBarSet *setLastWeek = new QBarSet("上周" + currentTransactionType);

        // 颜色统一使用蓝色系
            setThisWeek->setBrush(QColor("#3b6ea5")); // 深蓝
            setLastWeek->setBrush(QColor("#a0b8d5")); // 浅蓝
            setThisWeek->setPen(QPen(Qt::NoPen));
            setLastWeek->setPen(QPen(Qt::NoPen));

        // 根据类型决定读取哪个 JSON 字段
        QString valKey = (currentTransactionType == "支出") ? "dailyExpense" : "dailyIncome";

        for (int i = 0; i < 7; ++i) {
            double currentVal = (i < currentBars.size()) ? currentBars[i].toObject()[valKey].toDouble() : 0.0;
            double previousVal = (i < prevBars.size()) ? prevBars[i].toObject()[valKey].toDouble() : 0.0;
            *setThisWeek << currentVal;
            *setLastWeek << previousVal;
        }

        newBarSeries->append(setLastWeek);
        newBarSeries->append(setThisWeek);
        barChart->addSeries(newBarSeries);
        for (auto axis : barChart->axes()) newBarSeries->attachAxis(axis);
        // 重新绑定悬浮交互 (支持分辨本周/上周)
        connect(newBarSeries, &QBarSeries::hovered, this, [this](bool status, int index, QBarSet *barset) {
            if (status) {
                QStringList dayNames = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};
                QString periodLabel = barset->label(); // "本周" 或 "上周"

                QString tooltip = QString(
                    "<div style='font-family: DengXian; padding: 5px;'>"
                    "<b style='color:#333;'>%1 (%2)</b><br/>"
                    "金额: <span style='color:#3b6ea5; font-weight:bold;'>￥%3</span>"
                    "</div>"
                ).arg(dayNames.value(index)).arg(periodLabel).arg(barset->at(index), 0, 'f', 2);

                QToolTip::showText(QCursor::pos(), tooltip, barChartView);
            } else {
                QToolTip::hideText();
            }
        });

        barChart->setAnimationOptions(QChart::SeriesAnimations);

    // --- D. 更新底部周历按钮 ---
    QStringList weekDayNames = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};
    for (int i = 0; i < 7 && i < currentBars.size(); i++) {
        QJsonObject dayObj = currentBars[i].toObject();
        QDate dayDate = weekStart.addDays(i);
        double amount = dayObj[valKey].toDouble(); // 同样根据 key 读取

        QString btnText = QString("%1\n%2\n￥%3")
                            .arg(weekDayNames[i])
                            .arg(dayDate.toString("MM/dd"))
                            .arg(amount, 0, 'f', 2);

        if (i < dayButtons.size()) {
            dayButtons[i]->setText(btnText);
            dayButtons[i]->setChecked(dayDate == QDate::currentDate());
            dayButtons[i]->setFont(QFont("DengXian", 9));
        }
    }

    // --- E. 更新卡片总金额 ---
    double totalExp = current["weeklyExpenseTotal"].toDouble();
        double totalInc = current["weeklyIncomeTotal"].toDouble();

        QLabel *expLabel = expenseCard->findChild<QLabel*>("amountLabel");
        if (expLabel) expLabel->setText(QString("￥%1").arg(totalExp, 0, 'f', 2));

        QLabel *incLabel = incomeCard->findChild<QLabel*>("amountLabel");
        if (incLabel) incLabel->setText(QString("￥%1").arg(totalInc, 0, 'f', 2));

    // --- F. 更新评论 ---
    commentLabel->setText(current["comment"].toString());
    commentLabel->setFont(QFont("DengXian", 11));
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
    loadWeekData();
}


void WeekViewWidget::refreshData()
{
    loadWeekData();
}
