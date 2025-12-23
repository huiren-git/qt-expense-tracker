#ifndef DAYDETAILWIDGET_H
#define DAYDETAILWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonObject>

class DayDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DayDetailWidget(QWidget *parent = nullptr);

public slots:
    void loadDayData(const QString &date);
    void onAddClicked();
    void onEditClicked(int row);
    void onDeleteClicked(int row);
    void onBackClicked();

signals:
    void backToMainView();
    void addRecordRequested();
    void editRecordRequested(const QJsonObject &record);
    void deleteRecordRequested(qint64 id);
    void queryDayRecords(const QJsonObject &request);

private:
    void setupUI();
    void setupHeader();
    void setupTable();
    void updateDayData(const QJsonObject &data);

    QPushButton *backButton;
    QPushButton *addButton;
    QLabel *dateCardLabel;
    QTableWidget *recordsTable;
    QHBoxLayout *headerLayout;

    QString currentDate;
};

#endif // DAYDETAILWIDGET_H
