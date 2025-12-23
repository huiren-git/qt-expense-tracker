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

class RecordEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RecordEditDialog(QWidget *parent = nullptr, bool isEdit = false);
    ~RecordEditDialog();

    QJsonObject getRecordData() const;
    void setRecordData(const QJsonObject &record);

private slots:
    void onConfirmClicked();
    void onCancelClicked();

private:
    void setupUI();
    void setupForm();
    void setupDateTimeInputs();
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

    QLineEdit *counterpartyEdit;
    QLineEdit *productNameEdit;
    QLineEdit *remarkEdit;
    QLineEdit *sourceIdEdit;

    QPushButton *confirmButton;
    QPushButton *cancelButton;
};

#endif // RECORDEDITDIALOG_H
