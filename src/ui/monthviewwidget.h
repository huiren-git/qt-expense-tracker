#ifndef MONTHVIEWWIDGET_H
#define MONTHVIEWWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QCalendarWidget>
#include <QPushButton>
#include <QLabel>
#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QListWidget>
#include <QHBoxLayout>
#include <QJsonObject>

QT_CHARTS_USE_NAMESPACE

class MonthViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MonthViewWidget(QWidget *parent = nullptr);

public slots:
    void switchTransactionType(const QString &type);
    void onMonthChanged(int year, int month);
    void onDayClicked(const QString &date);

signals:
    void dayClicked(const QString &date);
    void queryMonthData(const QJsonObject &request);

private:
    void setupUI();
    void setupPieChart();
    void setupRankList();
    void setupCommentCard();
    void setupMonthSelector();
    void setupTransactionTypeCards();
    void setupCalendar();
    void loadMonthData();

    QComboBox *yearComboBox;
    QComboBox *monthComboBox;
    QHBoxLayout *monthSelectorLayout;
    QChartView *pieChartView;
    QListWidget *rankListWidget;
    QLabel *commentLabel;
    QPushButton *expenseCard;
    QPushButton *incomeCard;
    QCalendarWidget *calendarWidget;


    QString currentTransactionType;
    int currentYear;
    int currentMonth;
};

#endif // MONTHVIEWWIDGET_H
