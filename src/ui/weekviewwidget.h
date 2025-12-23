#ifndef WEEKVIEWWIDGET_H
#define WEEKVIEWWIDGET_H

#include <QWidget>
#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QJsonObject>
#include <QJsonArray>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDate>
#include <QJsonDocument>
#include <QPieSlice>
#include <QToolTip>

QT_CHARTS_USE_NAMESPACE

class WeekViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WeekViewWidget(QWidget *parent = nullptr);

public slots:
    void switchTransactionType(const QString &type);  // "支出" 或 "收入"
    void onWeekChanged(int year, int week);
    void onDayClicked(const QString &date);

signals:
    void dayClicked(const QString &date);
    void queryWeekData(const QJsonObject &request);

private:
    void setupUI();
    void setupPieChart();
    void setupRankList();
    void setupCommentCard();
    void setupWeekSelector();
    void setupTransactionTypeCards();
    void setupBarChart();
    void setupWeekCalendar();
    void loadWeekData();
    void updateWeekDisplay();
    void loadTestData();
    void updateWeekData(const QJsonObject &data);


    // 左侧组件
    QChartView *pieChartView;
    QListWidget *rankListWidget;
    QLabel *commentLabel;
    QPieSeries *pieSeries;  // 添加饼图系列指针，用于悬浮效果
    QMap<QPieSlice*, QJsonObject> sliceDataMap;  // 存储切片对应的数据

    // 右侧组件
    QPushButton *prevWeekButton;
    QPushButton *nextWeekButton;
    QLabel *weekRangeLabel;
    QPushButton *expenseCard;
    QPushButton *incomeCard;
    QChartView *barChartView;
    QTableWidget *weekCalendarTable;

    QString currentTransactionType;
    int currentYear;
    int currentWeek;
};

#endif // WEEKVIEWWIDGET_H
