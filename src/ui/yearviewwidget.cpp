#include "yearviewwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QJsonArray>
#include <QDate>
#include <QJsonObject>
#include <QJsonArray>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QLabel>
#include <QToolTip>
#include <QCursor>
#include <QBrush>
#include <QPen>
#include <QBarSeries>


YearViewWidget::YearViewWidget(QWidget *parent)
    : QWidget(parent)
    , currentTransactionType("支出")
{
    currentYear = QDate::currentDate().year();
    setupUI();
}

void YearViewWidget::setupUI()
{
    setStyleSheet("QWidget { background-color: #f5f8fb; }");
    this->setFont(QFont("DengXian", 12));

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 6, 20, 0);

    // 左侧：饼图、排行榜、评论
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(15);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    setupPieChart();
    leftLayout->addWidget(pieChartView);

    setupRankList();
    leftLayout->addWidget(rankListWidget);

    setupCommentCard();
    leftLayout->addWidget(commentLabel);
    leftLayout->addStretch(1);
    mainLayout->addLayout(leftLayout, 0);

    // 右侧：年份选择、收支卡片、柱状图
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(10);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    setupYearSelector(); // 改为箭头样式
    rightLayout->addWidget(yearSelectorWidget);

    setupTransactionTypeCards();
    QHBoxLayout *cardLayout = new QHBoxLayout();
    cardLayout->setSpacing(10);
    cardLayout->setContentsMargins(0, 5, 0, 5);
    cardLayout->addWidget(expenseCard);
    cardLayout->addWidget(incomeCard);
    cardLayout->addStretch();
    rightLayout->addLayout(cardLayout);

    setupBarChart();
    rightLayout->addWidget(barChartView, 1);

    mainLayout->addLayout(rightLayout, 1);
    switchTransactionType("支出");
}

void YearViewWidget::setupPieChart()
{
    pieChartView = new QChartView();
    pieChartView->setMinimumSize(300, 300);
    pieChartView->setMaximumSize(300, 300);
    pieChartView->setRenderHint(QPainter::Antialiasing);

    pieSeries = new QPieSeries();
    pieSeries->setHoleSize(0.35);

    connect(pieSeries, &QPieSeries::hovered, this, [this](QPieSlice *slice, bool state) {
        if (!slice) return;
        if (state) {
            slice->setExploded(true);
            slice->setExplodeDistanceFactor(0.05);
            slice->setPen(QPen(QColor("#3b6ea5"), 2));
            if (sliceDataMap.contains(slice)) {
                QJsonObject data = sliceDataMap[slice];
                QString tooltip = QString(
                    "<div style='font-family: Microsoft YaHei;'>"
                    "<b>类别:</b> %1<br/>"
                    "<b>占比:</b> <span style='color:#3b6ea5;'>%2%</span><br/>"
                    "<b>金额:</b> ￥%3"
                    "</div>"
                ).arg(data["category"].toString())
                 .arg(data["ratio"].toDouble() * 100, 0, 'f', 1)
                 .arg(data["totalAmount"].toDouble(), 0, 'f', 2);
                QToolTip::showText(QCursor::pos(), tooltip, pieChartView);
            }
        } else {
            slice->setExploded(false);
            slice->setPen(QPen(Qt::NoPen));
            QToolTip::hideText();
        }
    });

    QChart *chart = new QChart();
    chart->addSeries(pieSeries);
    chart->setTitle("分类占比");
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->legend()->setVisible(false);
    chart->setBackgroundBrush(QBrush(QColor("#ffffff")));
    pieChartView->setChart(chart);
}

void YearViewWidget::setupRankList()
{
    rankListWidget = new QListWidget();
    rankListWidget->setFixedHeight(200);
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
        "QListWidget::item:hover { background-color: transparent; }"
    );
}

void YearViewWidget::setupCommentCard()
{
    commentLabel = new QLabel("实用才是第一原则！");
    commentLabel->setFixedHeight(80);
    commentLabel->setFixedWidth(300);
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

void YearViewWidget::setupYearSelector()
{
    yearSelectorWidget = new QWidget();
    yearSelectorWidget->setStyleSheet("QWidget { background-color: transparent; }");
    QHBoxLayout *layout = new QHBoxLayout(yearSelectorWidget);
    layout->setContentsMargins(0, 5, 0, 5);
    layout->setSpacing(10);
    yearSelectorWidget->setFixedHeight(45);

    QPushButton *prevBtn = new QPushButton("◀");
    QPushButton *nextBtn = new QPushButton("▶");
    prevBtn->setFixedSize(36, 36);
    nextBtn->setFixedSize(36, 36);
    QString btnStyle =
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 15px; "
        "font-size: 18px; "
        "font-weight: bold; "
        "padding: 0; "
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
        "QPushButton:pressed { background-color: #2d5a8a; }";
    prevBtn->setStyleSheet(btnStyle);
    nextBtn->setStyleSheet(btnStyle);

    yearLabel = new QLabel(QString::number(currentYear) + "年");
    yearLabel->setStyleSheet("font-size: 14px; color: #333;");

    connect(prevBtn, &QPushButton::clicked, this, [this]() {
        currentYear--; yearLabel->setText(QString::number(currentYear) + "年");
        loadYearData();
    });
    connect(nextBtn, &QPushButton::clicked, this, [this]() {
        currentYear++; yearLabel->setText(QString::number(currentYear) + "年");
        loadYearData();
    });

    layout->addWidget(prevBtn);
    layout->addWidget(yearLabel);
    layout->addWidget(nextBtn);
    layout->addStretch();
}

void YearViewWidget::setupTransactionTypeCards()
{
    // 这里的逻辑应完全参考 WeekViewWidget::setupTransactionTypeCards
    // 关键点：使用内部 QVBoxLayout 放置两个 Label，并给 Label 设置 ObjectName 方便 switchTransactionType 切换颜色
    expenseCard = createCustomCard("支出");
    incomeCard = createCustomCard("收入");

    connect(expenseCard, &QPushButton::clicked, this, [this](){ switchTransactionType("支出"); });
    connect(incomeCard, &QPushButton::clicked, this, [this](){ switchTransactionType("收入"); });
}

// 辅助方法简化创建
QPushButton* YearViewWidget::createCustomCard(const QString &title) {
    QPushButton *card = new QPushButton();
    card->setFixedSize(160, 60);
    card->setCheckable(true);
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(15, 8, 15, 8);
    layout->setSpacing(0);

    QLabel *tLabel = new QLabel(title);
    tLabel->setObjectName("titleLabel");
    QLabel *vLabel = new QLabel("￥0.00");
    vLabel->setObjectName("amountLabel");
    vLabel->setStyleSheet("font-size: 18px; font-weight: bold; background-color: transparent;");

    layout->addWidget(tLabel);
    layout->addWidget(vLabel);
    layout->addStretch();
    return card;
}

void YearViewWidget::switchTransactionType(const QString &type)
{
    currentTransactionType = type;
    QString activeStyle =
        "QPushButton { background-color: white; border: 2px solid #3b6ea5; border-radius: 10px; }"
        "QLabel#titleLabel { color: #3b6ea5; background-color: transparent; font-size: 12px; }"
        "QLabel#amountLabel { color: #3b6ea5; background-color: transparent; }";
    QString inactiveStyle =
        "QPushButton { background-color: white; border: 2px solid #e0e8f0; border-radius: 10px; }"
        "QLabel#titleLabel { color: #999; background-color: transparent; font-size: 12px; }"
        "QLabel#amountLabel { color: #999; background-color: transparent; }";

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
    loadYearData();
}

void YearViewWidget::setupBarChart()
{
    barChartView = new QChartView();
    barChartView->setFixedHeight(440);
    barChartView->setRenderHint(QPainter::Antialiasing);

    QChart *chart = new QChart();
    chart->setTitle("年度收支趋势");
    chart->setBackgroundBrush(QBrush(QColor("#ffffff")));
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // X 轴：12 个月
    QStringList months;
    for (int i = 1; i <= 12; ++i)
        months << QString("%1月").arg(i);

    auto *axisX = new QBarCategoryAxis();
    axisX->append(months);
    chart->addAxis(axisX, Qt::AlignBottom);

    // Y 轴
    auto *axisY = new QValueAxis();
    axisY->setLabelFormat("%.0f");
    chart->addAxis(axisY, Qt::AlignLeft);

    // 只有一组柱子：本年
    barSeries = new QBarSeries();
    barSet = new QBarSet("本年");          // label 用在 tooltip 中
    barSet->setBrush(QColor("#3b6ea5"));
    barSet->setPen(QPen(Qt::NoPen));

    // 初始化 12 个 0
    for (int i = 0; i < 12; ++i)
        *barSet << 0;

    barSeries->append(barSet);
    chart->addSeries(barSeries);
    barSeries->attachAxis(axisX);
    barSeries->attachAxis(axisY);

    // 悬停提示
    connect(barSeries, &QBarSeries::hovered, this,
            [this](bool status, int index, QBarSet *set) {
        if (status && index >= 0 && index < set->count()) {
            // 1. 视觉反馈：高亮边框
            set->setPen(QPen(QColor("#1e3a5f"), 2));

            // 2. tooltip 内容
            int month = index + 1;
            double value = set->at(index);

            QString tooltip = QString(
                "<div style='font-family: Microsoft YaHei;'>"
                "<b>%1月</b> (%2)<br/>"
                "金额: <span style='color:#3b6ea5; font-size:14px;'>￥%3</span>"
                "</div>"
            ).arg(month)
             .arg(set->label())                 // "本年"
             .arg(value, 0, 'f', 2);

            QToolTip::showText(QCursor::pos(), tooltip, barChartView);
        } else {
            // 还原状态
            set->setPen(QPen(Qt::NoPen));
            QToolTip::hideText();
        }
    });

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // 为了让柱子 hover 边框变化更平滑
    chart->setAnimationOptions(QChart::AllAnimations);

    barChartView->setChart(chart);
}

void YearViewWidget::onYearChanged(int year)
{
    currentYear = year;
    loadYearData();
}

void YearViewWidget::loadYearData()
{
    // 模拟后端返回的 JSON
    QJsonObject mockResponse;
    mockResponse["year"] = currentYear;
    mockResponse["comment"] = (currentTransactionType == "支出") ? "年度支出控制在预期内。" : "年度收入稳步增长。";

    // 模拟卡片总额
    mockResponse["yearlyExpenseTotal"] = 45000.00;
    mockResponse["yearlyIncomeTotal"] = 62000.00;

    // 模拟 12 个月的数据
    QJsonArray months;
    for (int i = 0; i < 12; ++i) {
        QJsonObject m;
        // 根据支出/收入类型模拟不同数值
        m["total"] = (currentTransactionType == "支出") ? (2000 + i * 100) : (5000 + i * 50);
        months.append(m);
    }
    mockResponse["months"] = months;

    // 模拟饼图数据
    QJsonArray pie;
    QJsonObject p1;
    p1["category"] = (currentTransactionType == "支出") ? "餐饮美食" : "工资收入";
    p1["totalAmount"] = 12000.0;
    p1["ratio"] = 0.6;
    pie.append(p1);
    mockResponse["pie"] = pie;

    // 关键：必须调用更新函数
    updateYearData(mockResponse);
}

void YearViewWidget::updateYearData(const QJsonObject &json)
{
    // 1. 基础安全检查
    if (json.isEmpty()) return;

    // 假设 JSON 结构与周视图类似：
    // { "year": 2024, "yearlyExpenseTotal": 1200, "yearlyIncomeTotal": 2000, "months": [...], "pie": [...] }

    // --- A. 更新年份文字 ---
    int year = json["year"].toInt();
    yearLabel->setText(QString("%1 年").arg(year));

    // --- B. 更新收支卡片总额 ---
    double totalExp = json["yearlyExpenseTotal"].toDouble();
    double totalInc = json["yearlyIncomeTotal"].toDouble();

    QLabel *expVal = expenseCard->findChild<QLabel*>("amountLabel");
    if (expVal) expVal->setText(QString("￥%1").arg(totalExp, 0, 'f', 2));

    QLabel *incVal = incomeCard->findChild<QLabel*>("amountLabel");
    if (incVal) incVal->setText(QString("￥%1").arg(totalInc, 0, 'f', 2));

    // --- C. 更新饼图与排行榜 ---
    QJsonArray pieArray = json["pie"].toArray();
    if (pieChartView->chart()->series().count() > 0) {
        QPieSeries *series = qobject_cast<QPieSeries*>(pieChartView->chart()->series().at(0));
        series->clear();
        sliceDataMap.clear();
        rankListWidget->clear();

        int rank = 1;
        for (const QJsonValue &value : pieArray) {
            QJsonObject item = value.toObject();
            QString category = item["category"].toString();
            double amount = item["totalAmount"].toDouble();
            double ratio = item["ratio"].toDouble();

            QPieSlice *slice = series->append(category, amount);
            sliceDataMap[slice] = item; // 存入 Map 供 Hover 逻辑读取

            // 排行榜列表项
            QString rankText = QString("%1. %2 (%3%) - ￥%4")
                                .arg(rank++).arg(category)
                                .arg(ratio * 100, 0, 'f', 1)
                                .arg(amount, 0, 'f', 2);
            rankListWidget->addItem(new QListWidgetItem(rankText));
        }
    }

    // --- D. 更新柱状图 (12个月) ---
    QJsonArray monthsArray = json["months"].toArray();
    QChart *barChart = barChartView->chart();
    barChart->removeAllSeries();

    QBarSeries *newSeries = new QBarSeries();
    QBarSet *barSet = new QBarSet(currentTransactionType == "支出" ? "月支出" : "月收入");

    if (barSet) {
        barSet->setLabel(currentTransactionType == "支出" ? "月支出" : "月收入");
        barSet->remove(0, barSet->count()); // 清空
        QJsonArray monthsArray = json["months"].toArray();
        for (int i = 0; i < 12; ++i) {
            double val = (i < monthsArray.size())
                         ? monthsArray[i].toObject()["total"].toDouble()
                         : 0;
            *barSet << val;
        }
    }

    double maxVal = 0;
        for (int i = 0; i < 12; ++i) {
            double val = (i < monthsArray.size()) ? monthsArray[i].toObject()["total"].toDouble() : 0;
            barSet->replace(i, val); // 直接替换数据点，避免闪烁
            if (val > maxVal) maxVal = val;
        }
    if (auto *axisY = qobject_cast<QValueAxis*>(barChartView->chart()->axes(Qt::Vertical).first())) {
            axisY->setRange(0, maxVal * 1.2);
        }

    newSeries->append(barSet);
    barChart->addSeries(newSeries);
    newSeries->attachAxis(barChart->axes(Qt::Horizontal).first());
    newSeries->attachAxis(barChart->axes(Qt::Vertical).first());

    connect(newSeries, &QBarSeries::hovered, this, [this](bool status, int index, QBarSet *barset) {
        if (status && index >= 0) {
            // 1. 视觉反馈：显示 Tooltip
            QString tooltip = QString(
                "<div style='font-family: Microsoft YaHei;'>"
                "<b>%1月</b><br/>"
                "金额: <span style='color:#3b6ea5; font-size:14px;'>￥%2</span>"
                "</div>"
            ).arg(index + 1).arg(barset->at(index), 0, 'f', 2);

            QToolTip::showText(QCursor::pos(), tooltip, barChartView);

            // 2. 高亮单根柱子
            barset->setLabelColor(QColor("#3b6ea5"));
            //barset->setPen(QPen(QColor("#1e3a5f"), 2));
        } else {
            QToolTip::hideText();
        }
    });

    // --- E. 更新评论 ---
    commentLabel->setText(json["comment"].toString());
}
