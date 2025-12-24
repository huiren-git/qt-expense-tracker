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

/**
 * @brief 主窗口类
 *
 * 功能说明：
 * - 应用程序的主窗口，负责整体布局和视图切换
 * - 包含以下区域：
 *   1. 顶部栏：应用标题、帮助按钮、导入按钮
 *   2. 左侧栏：视图切换按钮（周视图、月视图、年视图）
 *   3. 主内容区：使用 QStackedWidget 管理多个视图
 *      - 空状态页面（无数据时显示）
 *      - 周度视图（WeekViewWidget）
 *      - 月度视图（MonthViewWidget）
 *      - 年度视图（YearViewWidget）
 *      - 单日详情视图（DayDetailWidget）
 *
 * 视图切换逻辑：
 * - 点击左侧栏按钮切换周/月/年视图
 * - 从周/月/年视图点击日期，切换到单日详情视图
 * - 从单日详情视图点击返回，回到之前的视图
 *
 * 数据导入功能：
 * - 点击导入按钮，打开文件选择对话框
 * - 支持导入支付宝 CSV 文件
 * - 调用数据层的导入接口（参考文档 2.1.1）
 *
 * 信号与槽连接：
 * - 各视图组件的信号连接到数据层的槽函数
 * - 数据层的信号连接到各视图组件的槽函数
 * - 视图之间的跳转通过信号槽机制实现
 */

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    /**
     * @brief 响应导入按钮点击
     *
     * 功能：
     * - 打开文件选择对话框
     * - 用户选择支付宝 CSV 文件
     * - 调用数据层的导入接口（onImportAlipayRequested）
     * - 显示导入进度和结果
     */
    void onImportClicked();
    void onHelpClicked();
    void onWeekViewClicked();
    void onMonthViewClicked();
    void onYearViewClicked();
    void onBackFromDetail();
    void showDayDetailView(const QString &date);

private:
    void setupUI();
    void setupTopBar();
    void setupSideBar();
    void setupContentArea();
    void showEmptyState();
    void showWeekView();
    void showMonthView();
    void showYearView();

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
    WeekViewWidget *weekViewWidget;        // 周度视图
    MonthViewWidget *monthViewWidget;      // 月度视图
    YearViewWidget *yearViewWidget;        // 年度视图
    DayDetailWidget *dayDetailWidget;

    // 当前选中的视图类型
    QString currentViewType;        // "week", "month", "year"
    QString currentTransactionType; // "支出", "收入"
};

#endif // MAINWINDOW_H
