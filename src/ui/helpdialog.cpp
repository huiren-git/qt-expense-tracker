#include "helpdialog.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("帮助");
    setFixedSize(600, 500);
    setStyleSheet("QDialog { background-color: #f0f4f8; }");

    setupUI();
}

void HelpDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    helpTextEdit = new QTextEdit();
    helpTextEdit->setReadOnly(true);
    helpTextEdit->setStyleSheet(
        "QTextEdit { "
        "background-color: white; "
        "border: 1px solid #d0d8e0; "
        "border-radius: 5px; "
        "padding: 10px; "
        "font-size: 14px; "
        "}"
    );

    QString helpText =
        "记账助手使用教程\n\n"
        "1. 数据导入\n"
        "   - 点击顶部栏的\"导入\"按钮，选择支付宝账单文件（CSV格式）\n"
        "   - 系统会自动解析并导入数据\n\n"
        "2. 视图切换\n"
        "   - 左侧栏提供\"周度\"、\"月度\"、\"年度\"三种视图\n"
        "   - 点击对应选项切换视图\n\n"
        "3. 收入/支出切换\n"
        "   - 在周度/月度/年度视图中，点击右上角的\"支出\"或\"收入\"卡片\n"
        "   - 切换后，图表和统计数据会相应更新\n\n"
        "4. 查看详情\n"
        "   - 在周度视图的周历或月度视图的日历中，点击具体日期\n"
        "   - 进入单日详情页面，查看该日所有交易记录\n\n"
        "5. 编辑记录\n"
        "   - 在单日详情页面，点击记录行的\"编辑\"按钮\n"
        "   - 修改记录信息后点击\"确定\"保存\n\n"
        "6. 删除记录\n"
        "   - 在单日详情页面，点击记录行的\"删除\"按钮\n"
        "   - 确认后删除该记录\n\n"
        "7. 新增记录\n"
        "   - 在单日详情页面，点击\"新增\"按钮\n"
        "   - 填写记录信息后点击\"确定\"保存";

    helpTextEdit->setPlainText(helpText);
    mainLayout->addWidget(helpTextEdit);

    closeButton = new QPushButton("关闭");
    closeButton->setFixedSize(80, 35);
    closeButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
    );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    mainLayout->addLayout(buttonLayout);
}
