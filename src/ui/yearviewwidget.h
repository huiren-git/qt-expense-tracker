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
    void updateYearData(const QJsonObject &);
    QPushButton* createCustomCard(const QString &);


    QWidget *yearSelectorWidget;
    QLabel *yearLabel;
    QComboBox *yearComboBox;
    QChartView *pieChartView;
    QPieSeries *pieSeries;
    QMap<QPieSlice*, QJsonObject> sliceDataMap;
    QListWidget *rankListWidget;
    QLabel *commentLabel;
    QPushButton *expenseCard;
    QPushButton *incomeCard;
    QChartView *barChartView;
    QBarSeries *barSeries = nullptr;
    QBarSet *bar;
    QBarSet *barSet = nullptr;

    QString currentTransactionType;
    int currentYear;
};

#endif // YEARVIEWWIDGET_H
