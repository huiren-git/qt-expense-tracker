#ifndef RECORDEDITDIALOG_H
#define RECORDEDITDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QFormLayout>
#include <QDateTimeEdit>
#include <QWidget>
#include <QJsonObject>
#include <QLabel>

/**
 * @brief 记录编辑对话框
 *
 * 功能说明：
 * - 用于新增或编辑单笔交易记录
 * - 支持两种模式：新增模式（isEdit=false）和编辑模式（isEdit=true）
 * - 包含以下表单字段：
 *   1. 交易日期时间：年、月、日、时、分、秒
 *   2. 交易类型：收入/支出（单选按钮）
 *   3. 分类：下拉框（根据交易类型动态加载）
 *   4. 交易方式：支付宝/现金/其他（单选按钮）
 *   5. 交易对方：文本输入
 *   6. 商品说明：文本输入
 *   7. 备注：文本输入
 *   8. 交易号：文本输入（可选，用于支付宝导入的数据）
 *
 * 数据格式：
 * 输入/输出均使用 QJsonObject，格式如下：
 * {
 *   "transactionDate": "2024-01-20 12:00:00",  // ISO 8601 格式
 *   "year": 2024,
 *   "month": 1,
 *   "week": 3,                                  // ISO 周数
 *   "amount": 120.50,                           // 金额（浮点数，保留两位小数）
 *   "transactionType": "支出",                  // "收入" 或 "支出"
 *   "category": "餐饮美食",                     // 分类名称（中文）
 *   "transactionMethod": "现金",                // "支付宝"、"现金"、"其他"
 *   "counterparty": "麦当劳",                   // 交易对方
 *   "productName": "午餐",                      // 商品说明
 *   "remark": "朋友聚餐",                       // 备注
 *   "sourceId": null                            // 支付宝交易号（可选）
 * }
 *
 * 分类枚举（参考文档）：
 * 支出类：餐饮美食、服饰装扮、日用百货、家居家装、数码电器、运动户外、
 *        美容美发、母婴亲子、宠物、交通出行、爱车养车、住房物业、酒店旅游、
 *        文化休闲、教育培训、医疗健康、生活服务、公共服务、商业服务、
 *        公益捐赠、互助保障、投资理财、保险、信用借还、充值缴费、其他
 *
 * 收入类：收入、转账红包、亲友代付、账户存取、退款、其他
 *
 * 交易方式枚举：
 * - "支付宝"
 * - "现金"
 * - "其他"
 */


class RecordEditDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     * @param isEdit 是否为编辑模式（true=编辑，false=新增）
     */
    explicit RecordEditDialog(QWidget *parent = nullptr, bool isEdit = false);
    ~RecordEditDialog();

    /**
     * @brief 获取表单数据
     * @return QJsonObject，包含所有表单字段的值
     *
     * 功能：
     * - 收集所有表单控件的值
     * - 构造符合 API 规范的 JSON 对象
     * - 计算 year、month、week 字段
     * - 返回完整的数据对象
     *
     * 注意：
     * - 金额字段需要转换为浮点数
     * - 日期时间需要格式化为 "YYYY-MM-DD HH:mm:ss" 格式
     * - 如果 sourceId 为空，应设置为 null
     */
    QJsonObject getRecordData() const;

    /**
     * @brief 设置表单数据（用于编辑模式）
     * @param record QJsonObject，包含要填充的数据
     *
     * 功能：
     * - 解析 JSON 对象
     * - 填充所有表单控件
     * - 设置交易类型、分类、交易方式等单选按钮和下拉框
     *
     * 注意：
     * - 需要解析日期时间字符串，分别填充到年、月、日、时、分、秒输入框
     * - 金额需要格式化为两位小数显示
     */
    void setRecordData(const QJsonObject &record);

private slots:
    /**
     * @brief 响应"确认"按钮点击
     *
     * 功能：
     * - 验证表单数据（必填字段、日期格式、金额格式等）
     * - 如果验证通过，调用 accept() 关闭对话框
     * - 如果验证失败，显示错误提示
     */
    void onConfirmClicked();

    /**
     * @brief 响应"取消"按钮点击
     *
     * 功能：
     * - 调用 reject() 关闭对话框
     * - 不保存任何数据
     */
    void onCancelClicked();

private:
    void setupUI();
    void setupForm();
    void setupDateTimeInputs();
    void setupAmountEdit();
    void setupTransactionTypeRadio();
    void setupCategoryComboBox();
    void setupTransactionMethodRadio();

    bool isEditMode;

    // 表单布局
    QFormLayout *formLayout;

    // 表单控件
    QWidget *dateTimeWidget;
    QLineEdit *yearEdit;
    QLineEdit *monthEdit;
    QLineEdit *dayEdit;
    QLineEdit *hourEdit;
    QLineEdit *minuteEdit;
    QLineEdit *secondEdit;

    QButtonGroup *transactionTypeGroup;
    QWidget *transactionTypeWidget;
    QRadioButton *expenseRadio;
    QRadioButton *incomeRadio;

    QComboBox *categoryComboBox;

    QButtonGroup *transactionMethodGroup;
    QWidget *transactionMethodWidget;
    QRadioButton *alipayRadio;
    QRadioButton *cashRadio;
    QRadioButton *othersRadio;

    QLineEdit *amountEdit;
    QLineEdit *counterpartyEdit;
    QLineEdit *productNameEdit;
    QLineEdit *remarkEdit;
    QLineEdit *sourceIdEdit;

    QPushButton *confirmButton;
    QPushButton *cancelButton;
};

#endif // RECORDEDITDIALOG_H
