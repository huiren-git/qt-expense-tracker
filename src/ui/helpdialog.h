#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>

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
