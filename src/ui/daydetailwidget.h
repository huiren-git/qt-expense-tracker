#ifndef DAYDETAILWIDGET_H
#define DAYDETAILWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonObject>

/**
 * @brief 单日详情组件
 *
 * 功能说明：
 * - 显示指定日期的所有交易记录详情
 * - 支持查看、新增、编辑、删除单笔交易
 * - 显示该日的总收入/总支出汇总
 * - 以表格形式展示所有交易记录，包含以下字段：
 *   - 交易时间
 *   - 金额
 *   - 交易类型（收入/支出）
 *   - 分类
 *   - 交易方式（支付宝/现金/其他）
 *   - 交易对方
 *   - 商品说明
 *   - 备注
 *
 * 数据接口：
 * - 槽函数 loadDayData()：加载指定日期的数据
 * - 信号 queryDayRecords()：向后端查询单日记录
 * - 信号 addRecordRequested()：请求新增记录
 * - 信号 editRecordRequested()：请求编辑记录
 * - 信号 deleteRecordRequested()：请求删除记录
 * - 信号 backToMainView()：返回主视图
 *
 * API接口规范（参考文档 2.2.4）：
 * 请求格式：
 * {
 *   "date": "2024-01-15"  // 日期格式 YYYY-MM-DD
 * }
 *
 * 响应格式：
 * {
 *   "operation": true,
 *   "dailyIncome": 0.00,
 *   "dailyExpense": 2600.00,
 *   "records": [
 *     {
 *       "id": 1001,  // 可选，本地主键
 *       "transactionDate": "2024-01-15 10:30:00",
 *       "amount": 2000.00,
 *       "transactionType": "支出",
 *       "category": "日用百货",
 *       "transactionMethod": "支付宝",
 *       "counterparty": "XX电商平台",
 *       "productName": "智能手机",
 *       "remark": "分期购买",
 *       "sourceId": "202401152030001234567890"  // 可选，支付宝交易号
 *     }
 *     // ... 更多记录
 *   ]
 * }
 *
 * 数据操作接口（参考文档 2.3）：
 * 1. 新增记录：addRecordRequested()
 *    请求格式：{
 *       "transactionDate": "2024-01-20 12:00:00",
 *       "year": 2024,
 *       "month": 1,
 *       "week": 3,
 *       "amount": 120.50,
 *       "transactionType": "支出",
 *       "category": "餐饮美食",
 *       "transactionMethod": "现金",
 *       "counterparty": "麦当劳",
 *       "productName": "午餐",
 *       "remark": "朋友聚餐",
 *       "sourceId": null
 *     }
 *
 * 2. 编辑记录：editRecordRequested()
 *    请求格式：{
 *       "id": 1001,  // 必需，用于定位记录
 *       "transactionDate": "2024-01-20 12:30:00",
 *       "revised": {
 *         "amount": 280.50,
 *         "category": "餐饮美食",
 *         "remark": "朋友聚餐补录",
 *         "transactionMethod": "支付宝"
 *       }
 *     }
 *
 * 3. 删除记录：deleteRecordRequested()
 *    请求格式：{
 *       "id": 1001  // 必需，用于定位记录
 *     }
 */


class WeekViewWidget;
class MonthViewWidget;
class YearViewWidget;

class DayDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DayDetailWidget(QWidget *parent = nullptr);
    void setViewWidgets(WeekViewWidget *weekView, MonthViewWidget *monthView, YearViewWidget *yearView);

public slots:

    /**
     * @brief 加载指定日期的数据
     * @param date 日期字符串，格式 "YYYY-MM-DD"（例如 "2024-01-15"）
     *
     * 功能：
     * - 更新 currentDate 状态
     * - 更新日期卡片显示
     * - 构造查询请求并发出 queryDayRecords() 信号
     * - 等待后端通过 sigDayRecordsReady() 信号返回数据
     */
    void loadDayData(const QString &date);

    /**
     * @brief 响应"新增"按钮点击
     *
     * 功能：
     * - 打开 RecordEditDialog 对话框（编辑模式为 false）
     * - 用户填写表单后，发出 addRecordRequested() 信号
     * - 等待后端通过 sigRecordAdded() 信号确认
     * - 刷新当前日期的数据
     */
    void onAddClicked();

    /**
     * @brief 响应"编辑"按钮点击
     * @param row 表格行号（0-based）
     *
     * 功能：
     * - 从表格中获取该行的记录数据
     * - 打开 RecordEditDialog 对话框（编辑模式为 true）
     * - 预填充表单数据
     * - 用户修改后，发出 editRecordRequested() 信号
     * - 等待后端通过 sigRecordUpdated() 信号确认
     * - 刷新当前日期的数据
     */
    void onEditClicked(int row);


    /**
     * @brief 响应"删除"按钮点击
     * @param row 表格行号（0-based）
     *
     * 功能：
     * - 从表格中获取该行的记录 ID
     * - 弹出确认对话框
     * - 用户确认后，发出 deleteRecordRequested() 信号
     * - 等待后端通过 sigRecordDeleted() 信号确认
     * - 刷新当前日期的数据
     */
    void onDeleteClicked(int row);

    /**
     * @brief 响应"返回"按钮点击
     *
     * 功能：
     * - 发出 backToMainView() 信号
     * - 通常连接到 MainWindow 的槽函数，切换回主视图
     */
    void onBackClicked();

signals:

    /**
     * @brief 查询单日记录信号
     * @param request QJsonObject，包含以下字段：
     *   - "date": QString，日期格式 "YYYY-MM-DD"
     *
     * 用途：
     * - 向数据层（后端）请求指定日期的所有交易记录
     * - 数据层应通过 sigDayRecordsReady() 信号返回数据
     *
     * 示例：
     * {
     *   "date": "2024-01-15"
     * }
     */
    void queryDayRecords(const QJsonObject &request); // 查询
    /**
     * @brief 新增记录请求信号
     * @param record QJsonObject，包含完整的交易记录数据
     *
     * 用途：
     * - 向数据层（后端）请求新增一条交易记录
     * - 数据层应通过 sigRecordAdded() 信号返回确认
     *
     * 数据格式：参考 API 接口规范 2.1.2
     */
    void addRecordRequested(const QJsonObject &record); // 新增

    /**
     * @brief 编辑记录请求信号
     * @param record QJsonObject，包含记录ID和修改后的字段
     *
     * 用途：
     * - 向数据层（后端）请求修改一条交易记录
     * - 数据层应通过 sigRecordUpdated() 信号返回更新后的完整记录
     *
     * 数据格式：参考 API 接口规范 2.3.1
     */
    void editRecordRequested(const QJsonObject &record); // 修改

    /**
     * @brief 删除记录请求信号
     * @param request QJsonObject，包含以下字段：
     *   - "id": int，记录的唯一标识符（本地主键）
     *
     * 用途：
     * - 向数据层（后端）请求删除一条交易记录
     * - 数据层应通过 sigRecordDeleted() 信号返回确认
     *
     * 示例：
     * {
     *   "id": 1001
     * }
     */
    void deleteRecordRequested(const QJsonObject &request); // 删除

    /**
     * @brief 返回主视图信号
     *
     * 用途：
     * - 当用户点击"返回"按钮时发出
     * - 通常连接到 MainWindow 的槽函数，切换回周/月/年视图
     */
    void backToMainView();

    void dataChanged();


private:
    void setupUI();
    void setupHeader();
    void setupTable();

    WeekViewWidget *weekViewWidget;
    MonthViewWidget *monthViewWidget;
    YearViewWidget *yearViewWidget;

    /**
     * @brief 更新单日数据到UI组件
     * @param data QJsonObject，符合 API 响应格式
     *
     * 功能：
     * - 解析后端返回的 JSON 数据
     * - 更新日期卡片（显示日期和总金额）
     * - 更新表格（显示所有交易记录）
     *
     * 数据格式：参考 API 接口规范 2.2.4
     */
    void updateDayData(const QJsonObject &data);

    QPushButton *backButton;
    QPushButton *addButton;
    QLabel *dateCardLabel;
    QTableWidget *recordsTable;
    QHBoxLayout *headerLayout;

    QString currentDate;
};

#endif // DAYDETAILWIDGET_H
