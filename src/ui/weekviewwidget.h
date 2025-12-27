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

/**
 * @brief 周度视图组件
 *
 * 功能说明：
 * - 显示指定周（ISO周数）的收支数据统计
 * - 支持通过箭头按钮切换上一周/下一周（默认显示当前周）
 * - 支持切换"支出"/"收入"两种交易类型
 * - 包含以下可视化组件：
 *   1. 饼图：显示分类占比
 *   2. 排行榜：显示分类金额排名
 *   3. 评论卡片：显示对应分类的评价文本
 *   4. 柱状图：显示本周与上周的每日金额对比
 *   5. 周历按钮：显示周一到周日的日期和金额，可点击查看详情
 *   6. 收支卡片：显示本周总收入/总支出
 *
 * 数据接口：
 * - 槽函数 onQueryWeekData()：接收后端返回的周度数据
 * - 信号 queryWeekData()：向后端请求周度数据
 * - 信号 dayClicked()：点击周历日期时触发，用于跳转到单日详情页
 *
 * API接口规范（参考文档 2.2.1）：
 * 请求格式：
 * {
 *   "week": 20,        // ISO周数 1-53
 *   "year": 2025,      // 年份
 *   "type": "income"   // "income" 或 "expense"
 * }
 *
 * 响应格式：
 * {
 *   "operation": true,
 *   "currentWeek": {
 *     "year": 2025,
 *     "week": 20,
 *     "weeklyIncomeTotal": 5200.00,
 *     "weeklyExpenseTotal": 4100.00,
 *     "dailyBars": [
 *       {
 *         "date": "2024-01-15",
 *         "hasRecords": false,
 *         "dailyAmount": 0.00,
 *         "dailyExpense": 0.00,
 *         "dailyIncome": 0.00
 *       }
 *       // ... 最多7条（周一到周日）
 *     ],
 *     "pie": [
 *       {
 *         "category": "餐饮美食",
 *         "totalAmount": 5200.00,
 *         "ratio": 1.00,
 *         "count": 2
 *       }
 *       // ... 分类数据
 *     ],
 *     "comment": "实用才是第一原则！"
 *   },
 *   "previousWeek": {
 *     // 同 currentWeek 结构
 *   }
 * }
 */


QT_CHARTS_USE_NAMESPACE

class WeekViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WeekViewWidget(QWidget *parent = nullptr);

public slots:
    /**
     * @brief 切换交易类型（支出/收入）
     * @param type "支出" 或 "收入"
     *
     * 功能：
     * - 切换当前显示的交易类型
     * - 更新饼图、排行榜、柱状图、周历等所有组件的数据
     * - 重新请求后端数据
     */
    void switchTransactionType(const QString &type);  // "支出" 或 "收入"

    /**
     * @brief 响应周数变化
     * @param year 年份（例如 2025）
     * @param week ISO周数（1-53）
     *
     * 功能：
     * - 更新当前显示的周数
     * - 重新请求该周的数据
     */
    void onWeekChanged(int year, int week);

    void refreshData();

    /**
    * @brief 响应日期点击事件
    * @param date 日期字符串，格式 "YYYY-MM-DD"（例如 "2024-01-15"）
    *
    * 功能：
    * - 处理从周历按钮触发的日期点击
    * - 通常用于跳转到单日详情页面
    */
    void onDayClicked(const QString &date);

signals:
    /**
     * @brief 日期点击信号
     * @param date 日期字符串，格式 "YYYY-MM-DD"
     *
     * 用途：
     * - 当用户点击周历中的某个日期按钮时发出
     * - 通常连接到 MainWindow 的 showDayDetailView() 槽函数
     */
    void dayClicked(const QString &date);

    /**
     * @brief 查询周度数据信号
     * @param request QJsonObject，包含以下字段：
     *   - "week": int，ISO周数（1-53）
     *   - "year": int，年份
     *   - "type": QString，"income" 或 "expense"
     *
     * 用途：
     * - 向数据层（后端）请求指定周的数据
     * - 数据层应通过 sigWeekDataReady() 信号返回数据
     *
     * 示例：
     * {
     *   "week": 20,
     *   "year": 2025,
     *   "type": "expense"
     * }
     */
    void queryWeekData(const QJsonObject &request);

private:
    void setupUI();
    void setupPieChart();
    void setupRankList();
    void setupCommentCard();
    void setupWeekSelector();
    void setupTransactionTypeCards();
    void setupBarChart();
    //void setupWeekCalendar();
    void setupWeekCalendarButtons();

    /**
     * @brief 加载周度数据（从后端）
     *
     * 功能：
     * - 构造请求 JSON
     * - 发出 queryWeekData() 信号
     */
    void loadWeekData();

    /**
     * @brief 更新周数显示文字
     *
     * 功能：
     * - 根据 currentYear 和 currentWeek 计算周一到周日的日期范围
     * - 更新 weekRangeLabel 的显示文本
     */
    void updateWeekDisplay();

    /**
     * @brief 加载测试数据（临时使用）
     *
     * 注意：正式版本应移除，改用 loadWeekData()
     */
    void loadTestData();

    QDate getMondayOfISOWeek(int, int);

    /**
     * @brief 更新周度数据到UI组件
     * @param data QJsonObject，符合 API 响应格式
     *
     * 功能：
     * - 解析后端返回的 JSON 数据
     * - 更新饼图、排行榜、评论卡片
     * - 更新柱状图（本周 vs 上周）
     * - 更新周历按钮（显示每日金额）
     * - 更新收支卡片（显示总金额）
     *
     * 数据格式：参考 API 接口规范 2.2.1
     */
    void updateWeekData(const QJsonObject &data);


    // 左侧组件
    QChartView *pieChartView;
    QListWidget *rankListWidget;
    QLabel *commentLabel;
    QPieSeries *pieSeries;  // 添加饼图系列指针，用于悬浮效果
    QMap<QPieSlice*, QJsonObject> sliceDataMap;  // 存储切片对应的数据

    // 右侧组件
    QWidget *selectorWidget;
    QPushButton *prevWeekButton;
    QPushButton *nextWeekButton;
    QLabel *weekRangeLabel;
    QPushButton *expenseCard;
    QPushButton *incomeCard;
    QChartView *barChartView;

    //QTableWidget *weekCalendarTable;

    // 日期按钮组相关
    QHBoxLayout *weekButtonsLayout;
    QList<QPushButton*> dayButtons;

    QString currentTransactionType;
    int currentYear;
    int currentWeek;
};

#endif // WEEKVIEWWIDGET_H
