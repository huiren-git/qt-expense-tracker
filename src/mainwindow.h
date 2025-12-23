#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

// 前向声明
class WeekViewWidget;
class MonthViewWidget;
class YearViewWidget;
class DayDetailWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onImportClicked();
    void onHelpClicked();
    void onWeekViewClicked();
    void onMonthViewClicked();
    void onYearViewClicked();
    void onBackFromDetail();

private:
    void setupUI();
    void setupTopBar();
    void setupSideBar();
    void setupContentArea();
    void showEmptyState();
    void showWeekView();
    void showMonthView();
    void showYearView();
    void showDayDetailView(const QString &date);

    Ui::MainWindow *ui;

    // 顶部栏组件
    QWidget *topBar;
    QLabel *appTitleLabel;
    QPushButton *helpButton;
    QPushButton *importButton;

    // 左侧栏组件
    QWidget *sideBar;
    QPushButton *weekButton;
    QPushButton *monthButton;
    QPushButton *yearButton;

    // 主内容区
    QStackedWidget *contentStack;
    QWidget *emptyStateWidget;      // 空状态页面
    QWidget *weekViewWidget;        // 周度视图
    QWidget *monthViewWidget;       // 月度视图
    QWidget *yearViewWidget;        // 年度视图
    DayDetailWidget *dayDetailWidget;        // 单日详情视图

    // 当前选中的视图类型
    QString currentViewType;        // "week", "month", "year"
    QString currentTransactionType; // "支出", "收入"
};

#endif // MAINWINDOW_H
