#ifndef MONTHVIEWWIDGET_H
#define MONTHVIEWWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QCalendarWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QPainter>

QT_CHARTS_USE_NAMESPACE

class MonthViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MonthViewWidget(QWidget *parent = nullptr);
    int getCurrentMonth() const { return currentMonth; };
    int getCurrentYear() const { return currentYear; };
    double getDayAmount(const QDate &date) const;
    int getCalendarMonthShown() const { return calendarWidget ? calendarWidget->monthShown() : currentMonth; };

signals:
    void dayClicked(const QString &date);

private slots:
    void switchTransactionType(const QString &type);
    void onMonthChanged(int year, int month);

private:
    void setupUI();
    void setupPieChart();
    void setupRankList();
    void setupCommentCard();
    void setupMonthSelector();
    void setupTransactionTypeCards();
    void setupCalendar();
    QFrame* createCalendarContainer();

    // 数据处理
    void loadTestData();
    void updateMonthData(const QJsonObject &json);

    // 状态变量
    int currentYear;
    int currentMonth;
    QString currentTransactionType;
    QMap<QPieSlice*, QJsonObject> sliceDataMap;
    QMap<QDate, double> m_dayAmounts; // 存储日期 -> 金额的映射

    // UI 组件
    QPieSeries *pieSeries;
    QChartView *pieChartView;
    QListWidget *rankListWidget;
    QLabel *commentLabel;

    QPushButton *expenseCard;
    QPushButton *incomeCard;
    QCalendarWidget *calendarWidget;

    QComboBox *yearComboBox;
    QComboBox *monthComboBox;
    QHBoxLayout *monthSelectorLayout;
};

#endif // MONTHVIEWWIDGET_H
