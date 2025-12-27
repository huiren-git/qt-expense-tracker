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

/**
 * @brief 月度视图组件
 *
 * 功能说明：
 * - 显示指定月份的收支数据统计
 * - 支持通过下拉框切换年份和月份
 * - 支持切换"支出"/"收入"两种交易类型
 * - 包含以下可视化组件：
 *   1. 饼图：显示分类占比
 *   2. 排行榜：显示分类金额排名
 *   3. 评论卡片：显示对应分类的评价文本
 *   4. 日历：显示当月每日金额，无记录日期变灰但可点击
 *   5. 收支卡片：显示本月总收入/总支出
 *
 * 数据接口：
 * - 槽函数 onQueryMonthData()：接收后端返回的月度数据
 * - 信号 dayClicked()：点击日历日期时触发，用于跳转到单日详情页
 *
 * API接口规范（参考文档 2.2.2）：
 * 请求格式：
 * {
 *   "month": 1,        // 月份 1-12
 *   "year": 2024,      // 年份
 *   "type": "income"   // "income" 或 "expense"
 * }
 *
 * 响应格式：
 * {
 *   "operation": true,
 *   "monthCalendar": [
 *     {
 *       "date": "2024-01-01",
 *       "hasRecords": true,
 *       "dailyAmount": 0.00,
 *       "dailyExpense": 0.00,
 *       "dailyIncome": 0.00
 *     }
 *     // ... 当月 1-31 日
 *   ],
 *   "monthlyIncomeTotal": 8000.00,
 *   "monthlyExpenseTotal": 6200.00,
 *   "pie": [
 *     {
 *       "category": "餐饮美食",
 *       "totalAmount": 8000.00,
 *       "ratio": 1.00,
 *       "count": 4
 *     }
 *     // ... 分类数据
 *   ],
 *   "comment": "实用才是第一原则！"
 * }
 */

class MonthViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MonthViewWidget(QWidget *parent = nullptr);

    /**
     * @brief 获取当前月份
     * @return 月份（1-12）
     */
    int getCurrentMonth() const { return currentMonth; };

    /**
     * @brief 获取当前年份
     * @return 年份（例如 2025）
     */
    int getCurrentYear() const { return currentYear; };

    /**
     * @brief 获取指定日期的金额
     * @param date 日期对象
     * @return 该日期的金额（根据当前交易类型返回收入或支出）
     *
     * 用途：
     * - 用于日历组件的日期绘制，显示每日金额
     */
    double getDayAmount(const QDate &date) const;

    /**
     * @brief 获取日历组件当前显示的月份
     * @return 月份（1-12）
     */
    int getCalendarMonthShown() const { return calendarWidget ? calendarWidget->monthShown() : currentMonth; };


public slots:
     void refreshData();

signals:

    /**
     * @brief 日期点击信号
     * @param date 日期字符串，格式 "YYYY-MM-DD"
     *
     * 用途：
     * - 当用户点击日历中的某个日期时发出
     * - 即使该日期无记录（hasRecords=false）也可以点击
     * - 通常连接到 MainWindow 的 showDayDetailView() 槽函数
     */
    void dayClicked(const QString &date);

private slots:

    /**
     * @brief 切换交易类型（支出/收入）
     * @param type "支出" 或 "收入"
     *
     * 功能：
     * - 切换当前显示的交易类型
     * - 更新饼图、排行榜、日历等所有组件的数据
     * - 重新请求后端数据
     */
    void switchTransactionType(const QString &type);

    /**
     * @brief 响应月份变化
     * @param year 年份（例如 2025）
     * @param month 月份（1-12）
     *
     * 功能：
     * - 更新当前显示的年份和月份
     * - 重新请求该月的数据
     */
    void onMonthChanged(int year, int month);

private:
    void setupUI();
    void setupPieChart();
    void setupRankList();
    void setupCommentCard();
    void setupMonthSelector();
    void setupTransactionTypeCards();
    void setupCalendar();
    void loadMonthData();
    QFrame* createCalendarContainer();

    // 数据处理
    /**
     * @brief 加载测试数据（临时使用）
     *
     * 注意：正式版本应移除，改用 loadMonthData()
     */
    void loadTestData();

    /**
     * @brief 更新月度数据到UI组件
     * @param json QJsonObject，符合 API 响应格式
     *
     * 功能：
     * - 解析后端返回的 JSON 数据
     * - 更新饼图、排行榜、评论卡片
     * - 更新日历显示（每日金额、是否有记录）
     * - 更新收支卡片（显示总金额）
     *
     * 数据格式：参考 API 接口规范 2.2.2
     */
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
