#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>

/**
 * @brief 帮助对话框
 *
 * 功能说明：
 * - 显示应用程序的使用说明和帮助信息
 * - 包含功能介绍、操作指南等内容
 * - 通过主窗口的帮助按钮打开
 *
 * 用途：
 * - 帮助用户了解如何使用应用程序
 * - 提供常见问题解答
 * - 显示版本信息和联系方式
 */

class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpDialog(QWidget *parent = nullptr);

private:
    void setupUI();
    QTextEdit *helpTextEdit;
    QPushButton *closeButton;
};

#endif // HELPDIALOG_H
