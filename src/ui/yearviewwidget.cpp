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

    setupYearSelector();
    rightLayout->addWidget(yearComboBox);

    setupTransactionTypeCards();
    QHBoxLayout *cardLayout = new QHBoxLayout();
    cardLayout->addWidget(expenseCard);
    cardLayout->addWidget(incomeCard);
    rightLayout->addLayout(cardLayout);

    setupBarChart();
    rightLayout->addWidget(barChartView);

    mainLayout->addLayout(rightLayout, 2);

    switchTransactionType("支出");
}

void YearViewWidget::setupPieChart()
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

void YearViewWidget::setupRankList()
{
    rankListWidget = new QListWidget();
    rankListWidget->setFixedHeight(200);
    rankListWidget->setStyleSheet(
        "QListWidget { "
        "background-color: white; "
        "border: 1px solid #d0d8e0; "
        "border-radius: 5px; "
        "}"
    );
}

void YearViewWidget::setupCommentCard()
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

void YearViewWidget::setupYearSelector()
{
    yearComboBox = new QComboBox();
    yearComboBox->setFixedWidth(120);
    for (int y = 2020; y <= 2030; y++) {
        yearComboBox->addItem(QString::number(y));
    }
    yearComboBox->setCurrentText(QString::number(currentYear));
    connect(yearComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        onYearChanged(yearComboBox->currentText().toInt());
    });
}

void YearViewWidget::setupTransactionTypeCards()
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

void YearViewWidget::switchTransactionType(const QString &type)
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
        "}";

    QString inactiveStyle =
        "QPushButton { "
        "background-color: #e0e8f0; "
        "color: #666; "
        "border: none; "
        "border-radius: 10px; "
        "text-align: left; "
        "padding: 10px; "
        "}";

    if (type == "支出") {
        expenseCard->setStyleSheet(activeStyle);
        incomeCard->setStyleSheet(inactiveStyle);
    } else {
        expenseCard->setStyleSheet(inactiveStyle);
        incomeCard->setStyleSheet(activeStyle);
    }

    loadYearData();
}

void YearViewWidget::setupBarChart()
{
    barChartView = new QChartView();
    barChartView->setFixedHeight(300);
    barChartView->setRenderHint(QPainter::Antialiasing);

    QBarSet *set = new QBarSet("月度金额");

    QBarSeries *series = new QBarSeries();
    series->append(set);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("月度对比");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories;
    for (int m = 1; m <= 12; m++) {
        categories << QString::number(m) + "月";
    }
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

void YearViewWidget::onYearChanged(int year)
{
    currentYear = year;
    loadYearData();
}

void YearViewWidget::loadYearData()
{
    // TODO: 调用后端接口
}
