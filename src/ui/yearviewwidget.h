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


/**
 * @brief 年度视图组件
 *
 * 功能说明：
 * - 显示指定年份的收支数据统计
 * - 支持通过下拉框切换年份（滚动条可切换年份）
 * - 支持切换"支出"/"收入"两种交易类型
 * - 包含以下可视化组件：
 *   1. 饼图：显示分类占比
 *   2. 排行榜：显示分类金额排名
 *   3. 评论卡片：显示对应分类的评价文本
 *   4. 柱状图：显示12个月的月度金额对比
 *   5. 收支卡片：显示本年度总收入/总支出
 *
 * 数据接口：
 * - 槽函数 onQueryYearData()：接收后端返回的年度数据
 * - 信号 queryYearData()：向后端请求年度数据
 *
 * API接口规范（参考文档 2.2.3）：
 * 请求格式：
 * {
 *   "year": 2024,      // 年份
 *   "type": "income"   // "income" 或 "expense"
 * }
 *
 * 响应格式：
 * {
 *   "operation": true,
 *   "year": 2024,
 *   "monthlyBars": [
 *     {
 *       "month": 1,
 *       "hasRecords": true,
 *       "monthlyAmountTotal": 8000.00,
 *       "monthlyExpenseTotal": 8000.00,
 *       "monthlyIncomeTotal": 0.00
 *     }
 *     // ... 1-12 月
 *   ],
 *   "annuallyIncomeTotal": 90000.00,
 *   "annuallyExpenseTotal": 75000.00,
 *   "pie": [
 *     {
 *       "category": "日用百货",
 *       "totalAmount": 10000.00,
 *       "ratio": 0.13,
 *       "count": 30
 *     }
 *     // ... 分类数据
 *   ],
 *   "comment": "实用才是第一原则！"
 * }
 */


class YearViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit YearViewWidget(QWidget *parent = nullptr);

public slots:

    /**
     * @brief 切换交易类型（支出/收入）
     * @param type "支出" 或 "收入"
     *
     * 功能：
     * - 切换当前显示的交易类型
     * - 更新饼图、排行榜、柱状图等所有组件的数据
     * - 重新请求后端数据
     */
    void switchTransactionType(const QString &type);

    /**
     * @brief 响应年份变化
     * @param year 年份（例如 2025）
     *
     * 功能：
     * - 更新当前显示的年份
     * - 重新请求该年的数据
     */
    void onYearChanged(int year);

    /**
     * @brief 刷新年度数据
     *
     * 功能：
     * - 重新加载当前年的数据
     * - 用于数据更新后刷新界面
     */
    void refreshData();

signals:

    /**
     * @brief 查询年度数据信号
     * @param request QJsonObject，包含以下字段：
     *   - "year": int，年份
     *   - "type": QString，"income" 或 "expense"
     *
     * 用途：
     * - 向数据层（后端）请求指定年份的数据
     * - 数据层应通过 sigYearDataReady() 信号返回数据
     *
     * 示例：
     * {
     *   "year": 2024,
     *   "type": "expense"
     * }
     */
    void queryYearData(const QJsonObject &request);

private:
    void setupUI();
    void setupPieChart();
    void setupRankList();
    void setupCommentCard();
    void setupYearSelector();
    void setupTransactionTypeCards();
    void setupBarChart();

    /**
     * @brief 加载年度数据（从后端）
     *
     * 功能：
     * - 构造请求 JSON
     * - 发出 queryYearData() 信号
     */
    void loadYearData();

    /**
     * @brief 更新年度数据到UI组件
     * @param json QJsonObject，符合 API 响应格式
     *
     * 功能：
     * - 解析后端返回的 JSON 数据
     * - 更新饼图、排行榜、评论卡片
     * - 更新柱状图（12个月的数据）
     * - 更新收支卡片（显示总金额）
     *
     * 数据格式：参考 API 接口规范 2.2.3
     */
    void updateYearData(const QJsonObject &);

    /**
     * @brief 创建自定义卡片按钮
     * @param title 卡片标题
     * @return QPushButton* 卡片按钮指针
     */
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
