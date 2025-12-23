#ifndef YEARVIEWWIDGET_H
#define YEARVIEWWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QListWidget>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QListWidget>
#include <QJsonObject>

QT_CHARTS_USE_NAMESPACE

class YearViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit YearViewWidget(QWidget *parent = nullptr);

public slots:
    void switchTransactionType(const QString &type);
    void onYearChanged(int year);

signals:
    void queryYearData(const QJsonObject &request);

private:
    void setupUI();
    void setupPieChart();
    void setupRankList();
    void setupCommentCard();
    void setupYearSelector();
    void setupTransactionTypeCards();
    void setupBarChart();
    void loadYearData();

    QComboBox *yearComboBox;
    QChartView *pieChartView;
    QListWidget *rankListWidget;
    QLabel *commentLabel;
    QPushButton *expenseCard;
    QPushButton *incomeCard;
    QChartView *barChartView;

    QString currentTransactionType;
    int currentYear;
};

#endif // YEARVIEWWIDGET_H
