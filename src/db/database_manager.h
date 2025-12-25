#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QSqlDatabase>
#include <QDateTime>
#include <QVariantList>

class DatabaseManager
{
public:
    static DatabaseManager& instance();

    // 创建表
    bool openDatabase();
    bool createTables();
    void insertDefaultTables();
    bool isReady() const;

    // 导入支付宝账单
    void importAlipayCsv(const QString &csvPath);

    /*数据库查询收支账单*/
    QSqlQuery getExpenseRecordsByYear(int year);  // 某年支出
    QSqlQuery getIncomeRecordsByYear(int year);  // 某年收入
    QSqlQuery getExpenseRecordsByMonth(int year, int month);  // 某月支出
    QSqlQuery getIncomeRecordsByMonth(int year, int month);  // 某月收入
    QSqlQuery getExpenseRecordsByWeek(int year, int week);  // 某周支出
    QSqlQuery getIncomeRecordsByWeek(int year, int week);  // 某周收入
    QSqlQuery getRecordsByDay(const QDateTime &date);  // 某天收支

    /*计算总收入或总支出*/
    QSqlQuery getTotalExpenseByYear(int year);  // 某年总支出
    QSqlQuery getTotalIncomeByYear(int year);  // 某年总收入
    QSqlQuery getTotalExpenseByMonth(int year, int month);  // 某月总支出
    QSqlQuery getTotalIncomeByMonth(int year, int month);  // 某月总收入
    QSqlQuery getTotalExpenseByWeek(int year, int week);  // 某周总支出
    QSqlQuery getTotalIncomeByWeek(int year, int week);  // 某周总收入
    QSqlQuery getTotalRecordsByDay(const QDateTime &date); // 某天总支出和总收入

    /*计算分类排行和占比 -> 返回有哪些类别及其对应的数量、总金额、总金额占比*/
    QSqlQuery getExpenseCategoryStatsByYear(int year);
    QSqlQuery getIncomeCategoryStatsByYear(int year);
    QSqlQuery getExpenseCategoryStatsByMonth(int year, int month);
    QSqlQuery getIncomeCategoryStatsByMonth(int year, int month);
    QSqlQuery getExpenseCategoryStatsByWeek(int year, int week);
    QSqlQuery getIncomeCategoryStatsByWeek(int year, int week);

    /*查询总金额占比最大的分类对应的评价*/
    QString getTopCategoryExpenseByYearWithComment(int year);
    QString getTopCategoryByMonthWithComment(int year, int month, const QString &transactionType);
    QString getTopCategoryByWeekWithComment(int year, int week, const QString &transactionType);


    /*收支记录增删改*/
    // 修改某条记录
    void updateRecord(int id, double amount,QString transaction_type, QString transactionDate, int categoryId, int methodId, QString counterparty, QString description,QString source_id, QString remark);
    // 新增一条记录
    void addRecord(double amount, QString transaction_type, QString transactionDate, int categoryId, int methodId, QString counterparty, QString description, QString source_id = "",QString remark="");
    // 删除某条记录
    void deleteRecord(int id);

    /*查询账单id*/
    // 根据交易单号查询
    int getBillIdByTransactionNumber(QString sourceId);
    // 根据交易时间查询消费订单
    int getExpenseBillIdByDate(QString transactionDate);
    // 根据交易时间查询收入订单
    int getIncomeBillIdByDate(QString transactionDate);

private:
    DatabaseManager();
    ~DatabaseManager();

    QSqlDatabase db;
    bool ready = false;
};

#endif // DATABASE_MANAGER_H
