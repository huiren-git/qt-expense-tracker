#include "monthviewwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDate>
#include <QJsonObject>
#include <QJsonArray>
#include <QToolTip>
#include <QFont>
#include <QFrame>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QTextCharFormat>

class CalendarDataDelegate : public QStyledItemDelegate {
    MonthViewWidget *m_view;
public:
    explicit CalendarDataDelegate(MonthViewWidget *parent)
        : QStyledItemDelegate(parent), m_view(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        // 1. 获取日期
        QDate date = index.data(Qt::UserRole).toDate();
        if (!date.isValid()) {
             // 兜底逻辑：如果 UserRole 没拿到，尝试从文本解析
             bool ok;
             int d = index.data(Qt::DisplayRole).toString().toInt(&ok);
             if (ok) date = QDate(m_view->getCurrentYear(), m_view->getCurrentMonth(), d);
        }

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        // 2. 状态判断
        const bool isSelected = option.state & QStyle::State_Selected;
        const bool isToday = (date == QDate::currentDate());
        // 重点：显式对比年和月，确保非本月变灰逻辑生效

        const bool isCurrentMonth = (date.month() == m_view->getCalendarMonthShown());
        //const bool isCurrentMonth = (date.month() == shownMonth);

        QRect rect = option.rect;

        // 3. 绘制背景
        if (isSelected) {
            painter->setBrush(QColor("#2f5d9a"));
            painter->setPen(Qt::NoPen);
            painter->drawRoundedRect(rect.adjusted(4, 4, -4, -4), 8, 8);
        } else if (isToday) {
            painter->setBrush(QColor(255, 200, 120, 100)); // 调低透明度
            painter->setPen(Qt::NoPen);
            painter->drawRoundedRect(rect.adjusted(4, 4, -4, -4), 8, 8);
        }

        // 4. 绘制日期数字（修复：之前代码漏掉了这一步导致字体看不见）
        QColor textColor;
        if (isSelected) {
            textColor = Qt::white;
        } else if (!isCurrentMonth) {
            textColor = QColor(150,150,150); // 非本月灰色
        } else if (isToday) {
            textColor = QColor("#d97706");
        } else {
            textColor = QColor("#333333");
        }

        painter->setPen(textColor);
        painter->setFont(QFont("DengXian", 11, QFont::Bold));
        // 将数字绘制在单元格上半部分，微调位置缩小与金额的间距
        QRect dayRect = rect;
        dayRect.setBottom(rect.top() + rect.height() / 2 + 3);
        painter->drawText(dayRect, Qt::AlignCenter, QString::number(date.day()));

        // 5. 绘制金额
        double amount = m_view->getDayAmount(date);
        if (amount > 0) {
            QColor amtColor;
            if (isSelected) {
                amtColor = QColor(235, 245, 255, 230);
            } else if (!isCurrentMonth) {
                amtColor = QColor(210, 210, 210); // 金额也要变灰
            } else {
                amtColor = QColor("#666666");
            }

            painter->setPen(amtColor);
            painter->setFont(QFont("DengXian", 9, QFont::Normal));
            // 将金额绘制在下半部分，微调位置靠近数字
            QRect amtRect = rect;
            amtRect.setTop(rect.top() + rect.height() / 2 - 2);
            painter->drawText(amtRect, Qt::AlignCenter, QString("￥%1").arg(amount, 0, 'f', 1));
        }

        painter->restore();
    }
};

MonthViewWidget::MonthViewWidget(QWidget *parent)
    : QWidget(parent)
    , currentTransactionType("支出")
{
    QDate today = QDate::currentDate();
    currentYear = today.year();
    currentMonth = today.month();

    setupUI();
    loadTestData();
}


double MonthViewWidget::getDayAmount(const QDate &date) const {
    return m_dayAmounts.value(date, 0.0);
}

void MonthViewWidget::setupUI()
{
    setStyleSheet("QWidget { background-color: #f5f8fb; }");
    this->setFont(QFont("DengXian", 12));

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(25);
    mainLayout->setContentsMargins(20, 15, 20, 15);

    // --- 左侧：饼图、排行榜、评论 ---
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(15);

    setupPieChart();
    setupRankList();
    setupCommentCard();

    leftLayout->addWidget(pieChartView);
    leftLayout->addWidget(rankListWidget);
    leftLayout->addWidget(commentLabel);
    // 注意：这里不加 Stretch，让左侧组件根据固定高度排列

    // --- 右侧：选择器、卡片、大型月历 ---
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(15);

    // 1. 年月选择器 (拉长拉高)
    setupMonthSelector();

    // 2. 收支汇总卡片
    setupTransactionTypeCards();
    QHBoxLayout *cardLayout = new QHBoxLayout();
    cardLayout->setSpacing(15);
    cardLayout->addWidget(expenseCard);
    cardLayout->addWidget(incomeCard);
    cardLayout->addStretch();

    // 3. 纯净日历容器
    QFrame *calendarContainer = new QFrame();
    calendarContainer->setStyleSheet(
        "QFrame { background-color: white; border: 1px solid #d0d8e0; border-radius: 12px; }"
    );
    QVBoxLayout *calLayout = new QVBoxLayout(calendarContainer);
    calLayout->setContentsMargins(5, 5, 5, 5);

    setupCalendar();
    calLayout->addWidget(calendarWidget);

    // 组装右侧
    rightLayout->addLayout(monthSelectorLayout);
    rightLayout->addLayout(cardLayout);
    rightLayout->addWidget(calendarContainer, 1); // Stretch 1 使日历自动拉伸至底部对齐

    mainLayout->addLayout(leftLayout, 0);
    mainLayout->addLayout(rightLayout, 1);

    switchTransactionType("支出");
}

void MonthViewWidget::setupPieChart()
{
    pieChartView = new QChartView();
    pieChartView->setMinimumSize(300, 300);
    pieChartView->setMaximumSize(300, 300);
    pieChartView->setRenderHint(QPainter::Antialiasing);

    pieSeries = new QPieSeries();
    pieSeries->setHoleSize(0.35);

    // 完美移植周度页面的悬浮动态
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

void MonthViewWidget::setupCalendar()
{
    calendarWidget = new QCalendarWidget();
    calendarWidget->setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);
    calendarWidget->setLocale(QLocale(QLocale::Chinese));
    calendarWidget->setFirstDayOfWeek(Qt::Sunday); // 周日放第一列
    calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader); // 移除左侧周序号
    calendarWidget->setGridVisible(false);
    calendarWidget->setNavigationBarVisible(false);
    calendarWidget->setCurrentPage(currentYear, currentMonth);

    if (auto *itemView = calendarWidget->findChild<QAbstractItemView*>("qt_calendar_calendarview")) {
        itemView->setItemDelegate(new CalendarDataDelegate(this));
        itemView->setMouseTracking(true);
        itemView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    QTextCharFormat defaultFormat;
        calendarWidget->setWeekdayTextFormat(Qt::Saturday, defaultFormat);
        calendarWidget->setWeekdayTextFormat(Qt::Sunday, defaultFormat);

        calendarWidget->setStyleSheet(
            "QCalendarWidget QWidget { background-color: white; border: none; }"
            "QCalendarWidget QAbstractItemView, QCalendarWidget QTableView { "
            "   background-color: white; "
            "   color: #2e2e2e; "
            "   selection-background-color: transparent; "
            "   selection-color: transparent; "
            "   outline: none; "
            "}"
            "QCalendarWidget QHeaderView::section { "
            "   background-color: #e6f0ff; "      /* 表头浅蓝 */
            "   color: #3b6ea5; "
            "   height: 45px; "
            "   font-weight: bold; "
            "   font-size: 16px; "
            "   border: none; "
            "   border-bottom: 1px solid #d6e4f5; "
            "}"
        );

    connect(calendarWidget, &QCalendarWidget::currentPageChanged,
            this, [this](int year, int month) {
                // 更新下拉框，但避免递归触发
                QSignalBlocker b1(yearComboBox), b2(monthComboBox);
                yearComboBox->setCurrentText(QString::number(year));
                monthComboBox->setCurrentText(QString::number(month));
                onMonthChanged(year, month); // 加载对应月份数据
            });

    connect(calendarWidget, &QCalendarWidget::clicked, this, [this](const QDate &date) {
        emit dayClicked(date.toString("yyyy-MM-dd")); // 跳转单日详情
    });
}

void MonthViewWidget::setupTransactionTypeCards()
{
    auto initCard = [this](QPushButton* &btn, const QString &title) {
        btn = new QPushButton();
        btn->setFixedSize(160, 60);
        btn->setCheckable(true);
        QVBoxLayout *layout = new QVBoxLayout(btn);
        layout->setContentsMargins(15, 8, 15, 8);
        layout->setSpacing(0);

        QLabel *tLabel = new QLabel(title);
        tLabel->setObjectName("titleLabel");
        QLabel *aLabel = new QLabel("￥0.00");
        aLabel->setObjectName("amountLabel");
        aLabel->setStyleSheet("font-size: 18px; font-weight: bold; background-color: transparent;");

        layout->addWidget(tLabel);
        layout->addWidget(aLabel);
        layout->addStretch();
    };

    initCard(expenseCard, "支出");
    initCard(incomeCard, "收入");

    connect(expenseCard, &QPushButton::clicked, this, [this](){ switchTransactionType("支出"); });
    connect(incomeCard, &QPushButton::clicked, this, [this](){ switchTransactionType("收入"); });
}

void MonthViewWidget::switchTransactionType(const QString &type)
{
    currentTransactionType = type;
    QString active = "QPushButton { background-color: white; border: 2px solid #3b6ea5; border-radius: 10px; } "
                     "QLabel#titleLabel, QLabel#amountLabel { color: #3b6ea5; background-color: transparent; }";
    QString inactive = "QPushButton { background-color: white; border: 2px solid #e0e8f0; border-radius: 10px; } "
                       "QLabel#titleLabel, QLabel#amountLabel { color: #999; background-color: transparent; }";

    expenseCard->setStyleSheet(type == "支出" ? active : inactive);
    incomeCard->setStyleSheet(type == "收入" ? active : inactive);
    expenseCard->setChecked(type == "支出");
    incomeCard->setChecked(type == "收入");

    loadTestData();
    calendarWidget->update();
}

void MonthViewWidget::updateMonthData(const QJsonObject &json)
{
    if (!json["operation"].toBool()) return;

    // 直接解析协议字段
    double expTotal = json["monthlyExpenseTotal"].toDouble();
    double incTotal = json["monthlyIncomeTotal"].toDouble();

    if(auto l = expenseCard->findChild<QLabel*>("amountLabel")) l->setText(QString("￥%1").arg(expTotal, 0, 'f', 2));
    if(auto l = incomeCard->findChild<QLabel*>("amountLabel")) l->setText(QString("￥%1").arg(incTotal, 0, 'f', 2));


    m_dayAmounts.clear();
        QJsonArray calArray = json["monthCalendar"].toArray();
        for (int i = 0; i < calArray.size(); ++i) {
            QJsonObject dayObj = calArray[i].toObject();
            QDate date = QDate::fromString(dayObj["date"].toString(), "yyyy-MM-dd");
            m_dayAmounts[date] = dayObj["dailyAmount"].toDouble();
        }


    pieSeries->clear();
    sliceDataMap.clear();
    rankListWidget->clear();

    QJsonArray pieArray = json["pie"].toArray();
    for (int i=0; i<pieArray.size(); ++i) {
        QJsonObject item = pieArray[i].toObject();
        QPieSlice *slice = pieSeries->append(item["category"].toString(), item["totalAmount"].toDouble());
        sliceDataMap[slice] = item;

        QString text = QString("%1. %2 (%3%) - ￥%4")
            .arg(i+1).arg(item["category"].toString())
            .arg(item["ratio"].toDouble()*100, 0, 'f', 1)
            .arg(item["totalAmount"].toDouble(), 0, 'f', 2);
        rankListWidget->addItem(text);
    }

    commentLabel->setText(json["comment"].toString());
}

void MonthViewWidget::setupRankList() {
    rankListWidget = new QListWidget();
    rankListWidget->setFixedHeight(200);
    rankListWidget->setFixedWidth(300);
    rankListWidget->setStyleSheet("QListWidget { background: white; border: 1px solid #d0d8e0; border-radius: 5px; }");
}

void MonthViewWidget::setupCommentCard() {
    commentLabel = new QLabel();
    commentLabel->setFixedHeight(80);
    commentLabel->setFixedWidth(300);
    commentLabel->setStyleSheet("QLabel { background: white; border: 2px dashed #3b6ea5; border-radius: 10px; padding: 10px; }");
    commentLabel->setWordWrap(true);
    commentLabel->setAlignment(Qt::AlignCenter);
}

void MonthViewWidget::setupMonthSelector() {
    monthSelectorLayout = new QHBoxLayout();
        monthSelectorLayout->setSpacing(5);

        // 极致简洁的下拉框样式
        QString minimalistCombo =
            "QComboBox { "
            "   border: none; border-bottom: 2px solid #d0d8e0; background: transparent; "
            "   padding: 2px 10px; min-width: 80px; font-weight: bold; color: #3b6ea5; "
            "}"
            "QComboBox:hover { border-bottom: 2px solid #3b6ea5; }"
            "QComboBox::drop-down { border: none; width: 20px; }"
            "QComboBox::down-arrow { image: none; border-left: 4px solid transparent; "
            "border-right: 4px solid transparent; border-top: 5px solid #3b6ea5; }";

        yearComboBox = new QComboBox();
        monthComboBox = new QComboBox();
        yearComboBox->setStyleSheet(minimalistCombo);
        monthComboBox->setStyleSheet(minimalistCombo);

        for (int y = 2020; y <= 2030; y++) yearComboBox->addItem(QString::number(y));
        for (int m = 1; m <= 12; m++) monthComboBox->addItem(QString::number(m));

        // “年”和“月”标签样式：无背景，淡灰色
        QString labelStyle = "QLabel { background: transparent; color: #666; font-weight: normal; margin-right: 15px; }";
        QLabel *yearLab = new QLabel("年"); yearLab->setStyleSheet(labelStyle);
        QLabel *monthLab = new QLabel("月"); monthLab->setStyleSheet(labelStyle);

        monthSelectorLayout->addWidget(yearComboBox);
        monthSelectorLayout->addWidget(yearLab);
        monthSelectorLayout->addWidget(monthComboBox);
        monthSelectorLayout->addWidget(monthLab);
        monthSelectorLayout->addStretch();

        auto updateFunc = [this]() {
            onMonthChanged(yearComboBox->currentText().toInt(), monthComboBox->currentText().toInt());
        };
        connect(yearComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, updateFunc);
        connect(monthComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, updateFunc);
}

void MonthViewWidget::onMonthChanged(int year, int month) {
    currentYear = year; currentMonth = month;
    {
        QSignalBlocker b1(yearComboBox), b2(monthComboBox);
        yearComboBox->setCurrentText(QString::number(year));
        monthComboBox->setCurrentText(QString::number(month));
    }
    calendarWidget->setCurrentPage(year, month);
    calendarWidget->setSelectedDate(QDate(year, month, 1));
    loadTestData();
    calendarWidget->update();
}

void MonthViewWidget::loadTestData() {
    QJsonObject resp;
        resp["operation"] = true;
        resp["monthlyIncomeTotal"] = 12500.0;
        resp["monthlyExpenseTotal"] = 4800.0;

        QJsonArray pie;
        if (currentTransactionType == "支出") {
            QJsonObject c1; c1["category"] = "餐饮美食"; c1["totalAmount"] = 1500.0; c1["ratio"] = 0.4;
            QJsonObject c2; c2["category"] = "日用百货"; c2["totalAmount"] = 1200.0; c2["ratio"] = 0.3;
            pie.append(c1); pie.append(c2);
            resp["comment"] = "本月餐饮开销正常，日用品有囤货现象。";
        } else {
            QJsonObject c1; c1["category"] = "工资收入"; c1["totalAmount"] = 10000.0; c1["ratio"] = 0.8;
            QJsonObject c2; c2["category"] = "基金分红"; c2["totalAmount"] = 2500.0; c2["ratio"] = 0.2;
            pie.append(c1); pie.append(c2);
            resp["comment"] = "薪然接受！本月理财收益超预期。";
        }
        resp["pie"] = pie;

        QJsonArray calArray;
        QDate first(currentYear, currentMonth, 1);
        int days = first.daysInMonth();
        for (int d = 1; d <= days; ++d) {
            QDate day(currentYear, currentMonth, d);
            QJsonObject obj;
            obj["date"] = day.toString("yyyy-MM-dd");
            if (currentTransactionType == "支出") {
                obj["dailyAmount"] = (d % 5 == 0) ? 120.5 : 35.2; // 示例：替换为真实数据
            } else {
                obj["dailyAmount"] = (d % 7 == 0) ? 300.0 : 80.0;
            }
            calArray.append(obj);
        }
        resp["monthCalendar"] = calArray;
        updateMonthData(resp);
        updateMonthData(resp);
}
