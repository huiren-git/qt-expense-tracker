#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QRegularExpression>

DatabaseManager::DatabaseManager()
{
}

DatabaseManager::~DatabaseManager()
{
    if (db.isOpen())
        db.close();
}

// 数据库实例
DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

// 打开数据库
bool DatabaseManager::openDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("app.db");

    if (!db.open()) {
        qDebug() << "数据库打开失败:" << db.lastError().text();
        ready = false;
        return false;
    } else {
        qDebug() << "数据库打开成功!";
    }

    QSqlQuery q;
    q.exec("PRAGMA foreign_keys = ON;");

    ready = true;
    return true;
}

// 创建表：分类表 / 交易方式表 / 账单表
bool DatabaseManager::createTables()
{
    if (!ready) return false;

    QSqlQuery query;

    // 分类表
    QString categorySql =
        "CREATE TABLE IF NOT EXISTS category ("
        " id INTEGER PRIMARY KEY,"
        " name TEXT NOT NULL,"
        " type TEXT CHECK(type IN ('income','expense')) NOT NULL"
        ");";

    if (!query.exec(categorySql)) {
        qDebug() << "创建 category 失败:" << query.lastError().text();
        return false;
    }

    // 交易方式表
    QString methodSql =
        "CREATE TABLE IF NOT EXISTS transaction_method ("
        " id INTEGER PRIMARY KEY,"
        " name TEXT NOT NULL"
        ");";

    if (!query.exec(methodSql)) {
        qDebug() << "创建 transaction_method 失败:" << query.lastError().text();
        return false;
    }

    // 账单表
    QString billSql =
        "CREATE TABLE IF NOT EXISTS bill_record ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " transaction_date TEXT,"
        " year INTEGER,"
        " month INTEGER,"
        " week INTEGER,"
        " amount REAL,"
        " transaction_type TEXT CHECK(transaction_type IN ('income','expense')),"
        " category_id INTEGER,"
        " transaction_method_id INTEGER,"
        " counterparty TEXT,"
        " description TEXT,"
        " remark TEXT,"
        " source_id TEXT UNIQUE,"
        " FOREIGN KEY(category_id) REFERENCES category(id),"
        " FOREIGN KEY(transaction_method_id) REFERENCES transaction_method(id)"
        ");";

    if (!query.exec(billSql)) {
        qDebug() << "创建 bill_record 失败:" << query.lastError().text();
        return false;
    }

    QString commentSql =
        " CREATE TABLE IF NOT EXISTS comment ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " category_id INTEGER NOT NULL,"
        " comment TEXT NOT NULL,"
        " FOREIGN KEY(category_id) REFERENCES category(id)"
        ");";

    if (!query.exec(commentSql)) {
        qDebug() << "创建 comment 失败:" << query.lastError().text();
        return false;
    }

    return true;
}

// 分类表、交易方式表和评论表插入记录
void DatabaseManager::insertDefaultTables()
{
    if (!ready) return;

    QStringList categories = {
        "餐饮美食","服饰装扮","日用百货","家居家装","数码电器",
        "运动户外","美容美发","母婴亲子","宠物","交通出行",
        "爱车养车","住房物业","酒店旅游","文化休闲","教育培训",
        "医疗健康","生活服务","公共服务","商业服务","公益捐赠",
        "互助保障","投资理财","保险","信用借还","充值缴费",
        "收入","转账红包","亲友代付","账户存取","退款","其他"
    };

    QStringList comments = {
        "吃的好才是真的好~", "体面人，讲究人。", "实用才是第一原则！", "助力每一个舒适小窝！",
        "更懂科技更懂你。", "生命在于运动！", "貌美如花，风流倜傥~", "家就是爱。",
        "感谢你的陪伴。", "路在脚下。", "好车子，自己养。", "平凡生活有保障。",
        "多出去看看吧！", "探索心灵的另一个空间。", "不断进步，不断探索。", "身体是革命的本钱。",
        "给生活加点料。", "公共的，惠民的。", "向成功的人士，致敬。", "大爱无疆。",
        "携手共进。", "跟风投降，叫风投。", "未雨绸缪！", "有点手段。",
        "价格就是服务。", "还有什么神奇的途径？", "薪然接受！", "谢谢老板，老板大气~",
        "好朋友一生一起走。", "必要的经济手段。", "我拒绝了你的拒绝。", "还有什么神奇的途径？"
    };

    QSqlQuery q;
    int id = 1;

    int category_ids[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 51, 53, 55, 57, 59, 61};

    for (auto &name : categories) {
        q.prepare("INSERT OR IGNORE INTO category(id,name,type) VALUES(?,?,?)");
        q.addBindValue(id++);
        q.addBindValue(name);
        q.addBindValue("expense");
        q.exec();

        q.prepare("INSERT OR IGNORE INTO category(id,name,type) VALUES(?,?,?)");
        q.addBindValue(id++);
        q.addBindValue(name);
        q.addBindValue("income");
        q.exec();
    }

    q.exec("INSERT OR IGNORE INTO transaction_method(id,name) VALUES(1,'cash');");
    q.exec("INSERT OR IGNORE INTO transaction_method(id,name) VALUES(2,'alipay');");
    q.exec("INSERT OR IGNORE INTO transaction_method(id,name) VALUES(3,'wechat');");

    for(int i = 0; i < comments.size(); i++) {
        q.prepare("INSERT OR IGNORE INTO comment(category_id, comment) VALUES (:category_id, :comment)");
        q.bindValue(":category_id", category_ids[i]);
        q.bindValue(":comment", comments[i]);
        q.exec();
    }
}

bool DatabaseManager::isReady() const
{
    return ready;
}

// 提取支付宝表中字段值
static QStringList parseSimpleAlipayCsvLine(const QString &line)
{
    QStringList cols;
    QString temp;
    int commaCount = 0;

    for(int i = 0; i < line.size(); ++i)
    {
        QChar c = line[i];

        // 前 11 列正常按逗号切
        if(c == ',' && commaCount < 11)
        {
            cols << temp.trimmed();
            temp.clear();
            commaCount++;
        }
        else
        {
            temp.append(c);
        }
    }

    // 剩下的是备注（可能含逗号）
    cols << temp.trimmed();

    return cols;
}


// 导入支付宝账单
void DatabaseManager::importAlipayCsv(const QString &csvPath)
{
    if(!ready){
        qDebug() << "数据库未初始化";
        return;
    }

    QFile file(csvPath);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "无法打开文件:" << csvPath;
        return;
    }

    QTextStream in(&file);
    in.setCodec("GBK");   // 支付宝 CSV 是 GBK

    bool dataStart = false;

    while(!in.atEnd())
    {
        QString line = in.readLine();

        // 读取所有的订单相关列
        if(line.startsWith("交易时间"))
        {
            dataStart = true;
            continue;
        }

        if(!dataStart) continue;
        if(line.trimmed().isEmpty()) continue;

        QStringList cols = parseSimpleAlipayCsvLine(line);

        if(cols.size() < 12) continue;

        // 裁剪出导入数据库的信息列
        QString time          = cols[0];
        QString categoryName  = cols[1];
        QString counterparty  = cols[2];
        QString description   = cols[4];
        QString incomeExpense = cols[5];   // 收入 / 支出 / 不计收支
        QString amount        = cols[6];
        QString methodName    = cols[7];   // 收/付款方式
        QString state         = cols[8];   // 交易状态
        QString orderNo = cols[9];
        orderNo.remove('"');
        orderNo = orderNo.trimmed();
        orderNo.remove(QRegularExpression("[^0-9]"));

        QString remark        = cols[11];

        // ---------- 筛选逻辑 ----------
        if(methodName.isEmpty()) continue;

        if(!(state == "支付成功" || state == "交易成功"))
            continue;

        if(!(incomeExpense == "收入" || incomeExpense == "支出"))
            continue;

        // ---------- 解析时间 ----------
        time = time.trimmed();
        time.replace(QRegularExpression("\\s+"), " ");

        // 尝试 yyyy-MM-dd HH:mm:ss
        QDateTime dt = QDateTime::fromString(time, "yyyy-MM-dd HH:mm:ss");

        // 如果失败，尝试 yyyy/M/d H:mm:ss
        if(!dt.isValid())
            dt = QDateTime::fromString(time, "yyyy/M/d H:mm:ss");

        // 如果再失败，尝试 yyyy/M/d H:mm
        if(!dt.isValid())
            dt = QDateTime::fromString(time, "yyyy/M/d H:mm");

        if(!dt.isValid()){
            qDebug() << "时间解析失败:" << time;
            continue;   // 防止脏数据继续插入
        }


        int year  = dt.date().year();
        int month = dt.date().month();
        int week  = dt.date().weekNumber();

        // ---------- transaction_type ----------
        QString type = (incomeExpense == "收入") ? "income" : "expense";

        // ---------- 分类 id ----------
        QSqlQuery q;
        q.prepare("SELECT id FROM category WHERE name=? AND type=?");
        q.addBindValue(categoryName);
        q.addBindValue(type);
        q.exec();

        int categoryId = -1;
        if(q.next())
            categoryId = q.value(0).toInt();

        // ---------- 插入 bill_record ----------
        QSqlQuery ins;
        ins.prepare(
            "INSERT OR IGNORE INTO bill_record("
            "transaction_date, year, month, week, amount, transaction_type,"
            "category_id, transaction_method_id, counterparty, description, remark, source_id"
            ") VALUES (?,?,?,?,?,?,?,?,?,?,?,?)"
            );

        ins.addBindValue(dt.toString("yyyy-MM-dd HH:mm:ss"));
        ins.addBindValue(year);
        ins.addBindValue(month);
        ins.addBindValue(week);
        ins.addBindValue(amount.toDouble());
        ins.addBindValue(type);
        ins.addBindValue(categoryId);
        ins.addBindValue(2);
        ins.addBindValue(counterparty);
        ins.addBindValue(description);
        ins.addBindValue(remark);
        ins.addBindValue(orderNo);

        if(!ins.exec()){
            qDebug() << "插入失败:" << ins.lastError();
        }
    }

    qDebug() << "支付宝账单导入完成!";
}

// 筛选某年的所有支出记录
QSqlQuery DatabaseManager::getExpenseRecordsByYear(int year)
{
    QSqlQuery query;
    query.prepare(
        "SELECT * FROM bill_record "
        "WHERE year = :year "
        "AND transaction_type = 'expense';"
    );
    query.bindValue(":year", year);
    query.exec();

    return query;
}

// 筛选某年的所有收入记录
QSqlQuery DatabaseManager::getIncomeRecordsByYear(int year)
{
    QSqlQuery query;
    query.prepare(
        "SELECT * FROM bill_record "
        "WHERE year = :year "
        "AND transaction_type = 'income';"
    );
    query.bindValue(":year", year);
    query.exec();

    return query;
}

// 筛选某月的所有支出记录
QSqlQuery DatabaseManager::getExpenseRecordsByMonth(int year, int month)
{
    QSqlQuery query;
    query.prepare(
        "SELECT * FROM bill_record "
        "WHERE year = :year "
        "AND month = :month "
        "AND transaction_type = 'expense';"
    );
    query.bindValue(":year", year);
    query.bindValue(":month", month);
    query.exec();

    return query;
}

// 筛选某月的所有收入记录
QSqlQuery DatabaseManager::getIncomeRecordsByMonth(int year, int month)
{
    QSqlQuery query;
    query.prepare(
        "SELECT * FROM bill_record "
        "WHERE year = :year "
        "AND month = :month "
        "AND transaction_type = 'income';"
    );
    query.bindValue(":year", year);
    query.bindValue(":month", month);
    query.exec();

    return query;
}

// 筛选某周的所有支出记录
QSqlQuery DatabaseManager::getExpenseRecordsByWeek(int year, int week)
{
    QSqlQuery query;
    query.prepare(
        "SELECT * FROM bill_record "
        "WHERE year = :year "
        "AND week = :week "
        "AND transaction_type = 'expense';"
    );
    query.bindValue(":year", year);
    query.bindValue(":week", week);
    query.exec();

    return query;
}

// 筛选某周的所有收入记录
QSqlQuery DatabaseManager::getIncomeRecordsByWeek(int year, int week)
{
    QSqlQuery query;
    query.prepare(
        "SELECT * FROM bill_record "
        "WHERE year = :year "
        "AND week = :week "
        "AND transaction_type = 'income';"
    );
    query.bindValue(":year", year);
    query.bindValue(":week", week);
    query.exec();

    return query;
}

// 筛选某天的所有支出和收入记录
QSqlQuery DatabaseManager::getRecordsByDay(QString date)
{
    //注意：因日期无法调用记录修改
    //原：WHERE transaction_date = :date;
    //字符串加%
    QSqlQuery query;
    query.prepare(
        "SELECT * FROM bill_record "
        "WHERE transaction_date LIKE :date;"
    );
    query.bindValue(":date", date+"%");
    query.exec();

    return query;
}

// 筛选某年的总支出
QSqlQuery DatabaseManager::getTotalExpenseByYear(int year)
{
    QSqlQuery query;
    query.prepare(
        "SELECT SUM(amount) AS total_expense FROM bill_record "
        "WHERE year = :year "
        "AND transaction_type = 'expense';"
    );
    query.bindValue(":year", year);
    query.exec();

    return query;
}

// 筛选某年的总收入
QSqlQuery DatabaseManager::getTotalIncomeByYear(int year)
{
    QSqlQuery query;
    query.prepare(
        "SELECT SUM(amount) AS total_income FROM bill_record "
        "WHERE year = :year "
        "AND transaction_type = 'income';"
    );
    query.bindValue(":year", year);
    query.exec();

    return query;
}

// 筛选某月的总支出
QSqlQuery DatabaseManager::getTotalExpenseByMonth(int year, int month)
{
    QSqlQuery query;
    query.prepare(
        "SELECT SUM(amount) AS total_expense FROM bill_record "
        "WHERE year = :year "
        "AND month = :month "
        "AND transaction_type = 'expense';"
    );
    query.bindValue(":year", year);
    query.bindValue(":month", month);
    query.exec();

    return query;
}

// 筛选某月的总收入
QSqlQuery DatabaseManager::getTotalIncomeByMonth(int year, int month)
{
    QSqlQuery query;
    query.prepare(
        "SELECT SUM(amount) AS total_income FROM bill_record "
        "WHERE year = :year "
        "AND month = :month "
        "AND transaction_type = 'income';"
    );
    query.bindValue(":year", year);
    query.bindValue(":month", month);
    query.exec();

    return query;
}

// 筛选某周的总支出
QSqlQuery DatabaseManager::getTotalExpenseByWeek(int year, int week)
{
    QSqlQuery query;
    query.prepare(
        "SELECT SUM(amount) AS total_expense FROM bill_record "
        "WHERE year = :year "
        "AND week = :week "
        "AND transaction_type = 'expense';"
    );
    query.bindValue(":year", year);
    query.bindValue(":week", week);
    query.exec();

    return query;
}

// 筛选某周的总收入
QSqlQuery DatabaseManager::getTotalIncomeByWeek(int year, int week)
{
    QSqlQuery query;
    query.prepare(
        "SELECT SUM(amount) AS total_income FROM bill_record "
        "WHERE year = :year "
        "AND week = :week "
        "AND transaction_type = 'income';"
    );
    query.bindValue(":year", year);
    query.bindValue(":week", week);
    query.exec();

    return query;
}

// 筛选某天的总支出和总收入
QSqlQuery DatabaseManager::getTotalRecordsByDay(QString date)
{
    QSqlQuery query;
    // 此处更改
    // 原：WHERE transaction_date = :date;
    //字符串加%
    query.prepare(
        "SELECT "
        "SUM(CASE WHEN transaction_type = 'expense' THEN amount ELSE 0 END) AS total_expense, "
        "SUM(CASE WHEN transaction_type = 'income' THEN amount ELSE 0 END) AS total_income "
        "FROM bill_record "
        "WHERE transaction_date LIKE :date;"
    );
    query.bindValue(":date", date+"%");
    query.exec();

    return query;
}

// 查询某年的支出分类统计
QSqlQuery DatabaseManager::getExpenseCategoryStatsByYear(int year)
{
    QSqlQuery query;
    query.prepare(
        "SELECT c.name, COUNT(b.id) AS bill_count, SUM(b.amount) AS total_amount "
        "FROM bill_record b "
        "JOIN category c ON b.category_id = c.id "
        "WHERE year = :year "
        "AND b.transaction_type = 'expense' "
        "GROUP BY c.name;"
    );
    query.bindValue(":year", year);
    query.exec();

    return query;
}

// 查询某年的收入分类统计
QSqlQuery DatabaseManager::getIncomeCategoryStatsByYear(int year)
{
    QSqlQuery query;
    query.prepare(
        "SELECT c.name, COUNT(b.id) AS bill_count, SUM(b.amount) AS total_amount "
        "FROM bill_record b "
        "JOIN category c ON b.category_id = c.id "
        "WHERE year = :year "
        "AND b.transaction_type = 'income' "
        "GROUP BY c.name;"
    );
    query.bindValue(":year", year);
    query.exec();

    return query;
}

// 查询某月的支出分类统计
QSqlQuery DatabaseManager::getExpenseCategoryStatsByMonth(int year, int month)
{
    QSqlQuery query;
    query.prepare(
        "SELECT c.name, COUNT(b.id) AS bill_count, SUM(b.amount) AS total_amount "
        "FROM bill_record b "
        "JOIN category c ON b.category_id = c.id "
        "WHERE year = :year "
        "AND month = :month "
        "AND b.transaction_type = 'expense' "
        "GROUP BY c.name;"
    );
    query.bindValue(":year", year);
    query.bindValue(":month", month);
    query.exec();

    return query;
}

// 查询某月的收入分类统计
QSqlQuery DatabaseManager::getIncomeCategoryStatsByMonth(int year, int month)
{
    QSqlQuery query;
    query.prepare(
        "SELECT c.name, COUNT(b.id) AS bill_count, SUM(b.amount) AS total_amount "
        "FROM bill_record b "
        "JOIN category c ON b.category_id = c.id "
        "WHERE year = :year "
        "AND month = :month "
        "AND b.transaction_type = 'income' "
        "GROUP BY c.name;"
    );
    query.bindValue(":year", year);
    query.bindValue(":month", month);
    query.exec();

    return query;
}

// 查询某周的支出分类统计
QSqlQuery DatabaseManager::getExpenseCategoryStatsByWeek(int year, int week)
{
    QSqlQuery query;
    query.prepare(
        "SELECT c.name, COUNT(b.id) AS bill_count, SUM(b.amount) AS total_amount "
        "FROM bill_record b "
        "JOIN category c ON b.category_id = c.id "
        "WHERE year = :year "
        "AND week = :week "
        "AND b.transaction_type = 'expense' "
        "GROUP BY c.name;"
    );
    query.bindValue(":year", QString::number(year));
    query.bindValue(":week", QString::number(week).rightJustified(2, '0'));
    query.exec();

    return query;
}

// 查询某周的收入分类统计
QSqlQuery DatabaseManager::getIncomeCategoryStatsByWeek(int year, int week)
{
    QSqlQuery query;
    query.prepare(
        "SELECT c.name, COUNT(b.id) AS bill_count, SUM(b.amount) AS total_amount "
        "FROM bill_record b "
        "JOIN category c ON b.category_id = c.id "
        "WHERE year = :year "
        "AND week = :week "
        "AND b.transaction_type = 'income' "
        "GROUP BY c.name;"
    );
    query.bindValue(":year", year);
    query.bindValue(":week", week);
    query.exec();

    return query;
}

// 查询某年的总支出金额评价
QString DatabaseManager::getTopCategoryByYearWithComment(int year, const QString &transactionType)
{
    // 查询总金额（支出或收入）
        QSqlQuery totalQuery;
        totalQuery.prepare(
            "SELECT SUM(amount) AS total_amount FROM bill_record "
            "WHERE year = :year "
            "AND transaction_type = :transaction_type;"
        );
        totalQuery.bindValue(":year", QString::number(year));
        totalQuery.bindValue(":transaction_type", transactionType);
        totalQuery.exec();

        double totalAmount = 0;
        if (totalQuery.next()) {
            totalAmount = totalQuery.value("total_amount").toDouble();
        }

        // 查询每个分类的统计（支出或收入）
        QSqlQuery query;
        query.prepare(
            "SELECT c.name, COUNT(b.id) AS bill_count, SUM(b.amount) AS total_amount "
            "FROM bill_record b "
            "JOIN category c ON b.category_id = c.id "
            "WHERE year = :year "
            "AND b.transaction_type = :transaction_type "
            "GROUP BY c.name;"
        );
        query.bindValue(":year", QString::number(year));
        query.bindValue(":transaction_type", transactionType);
        query.exec();

        // 查找占比最大的分类
        double maxPercentage = 0;
        QString topCategoryName;
        double topCategoryTotal = 0;
        while (query.next()) {
            QString categoryName = query.value("name").toString();
            double categoryTotal = query.value("total_amount").toDouble();

            // 计算占比
            double percentage = (totalAmount > 0) ? (categoryTotal / totalAmount) * 100 : 0;

            // 更新占比最大的分类
            if (percentage > maxPercentage) {
                maxPercentage = percentage;
                topCategoryName = categoryName;
                topCategoryTotal = categoryTotal;
            }
        }

        // 获取对应的评论
        QString comment;
        if (!topCategoryName.isEmpty()) {
            QSqlQuery commentQuery;

            commentQuery.prepare(
                "SELECT comment FROM comment "
                "WHERE category_id = (SELECT id FROM category WHERE name = :category_name "
                "AND type = :transaction_type);"
            );
            commentQuery.bindValue(":category_name", topCategoryName);
            //commentQuery.bindValue(":transaction_type", transactionType);
            //暂且这样获得评论
            commentQuery.bindValue(":transaction_type", "expense");
            if(!commentQuery.exec()){

                qDebug()<< "获取年评论错误：" << commentQuery.lastError();
            }

            if (commentQuery.next()) {
                comment = commentQuery.value("comment").toString();
            }
        }


    qDebug() << "top分类: " << topCategoryName;
    qDebug() << "总成交量: " << topCategoryTotal;
    qDebug() << "金额占比: " << maxPercentage << "%";
    qDebug() << "评论: " << comment;

    return comment;
}


// 按月筛选总金额占比最大的分类的评论
QString DatabaseManager::getTopCategoryByMonthWithComment(int year, int month, const QString &transactionType)
{
    // 查询某月的总金额
    QSqlQuery totalQuery;
    totalQuery.prepare(
        "SELECT SUM(amount) AS total_amount FROM bill_record "
        "WHERE year = :year "
        "AND month = :month "
        "AND transaction_type = :transaction_type;"
    );
    totalQuery.bindValue(":year", QString::number(year));
    totalQuery.bindValue(":month", QString::number(month).rightJustified(2, '0'));  // 保证月份是两位数
    totalQuery.bindValue(":transaction_type", transactionType);
    totalQuery.exec();

    double totalAmount = 0;
    if (totalQuery.next()) {
        totalAmount = totalQuery.value("total_amount").toDouble();
    }

    // 查询每个分类的金额总和
    QSqlQuery query;
    query.prepare(
        "SELECT c.name, COUNT(b.id) AS bill_count, SUM(b.amount) AS total_amount "
        "FROM bill_record b "
        "JOIN category c ON b.category_id = c.id "
        "WHERE year = :year "
        "AND month = :month "
        "AND b.transaction_type = :transaction_type "
        "GROUP BY c.name;"
    );
    query.bindValue(":year", QString::number(year));
    query.bindValue(":month", QString::number(month).rightJustified(2, '0'));
    query.bindValue(":transaction_type", transactionType);
    query.exec();

    // 查找占比最大的分类
    double maxPercentage = 0;
    QString topCategoryName;
    double topCategoryTotal = 0;
    while (query.next()) {
        QString categoryName = query.value("name").toString();
        double categoryTotal = query.value("total_amount").toDouble();

        // 计算占比
        double percentage = (totalAmount > 0) ? (categoryTotal / totalAmount) * 100 : 0;

        // 更新占比最大的分类
        if (percentage > maxPercentage) {
            maxPercentage = percentage;
            topCategoryName = categoryName;
            topCategoryTotal = categoryTotal;
        }
    }

    // 获取对应的评论
    QString comment;
    if (!topCategoryName.isEmpty()) {
        QSqlQuery commentQuery;
        commentQuery.prepare(
            "SELECT comment FROM comment "
            "WHERE category_id = (SELECT id FROM category WHERE name = :category_name "
            "AND type = :transaction_type);"
        );
        commentQuery.bindValue(":category_name", topCategoryName);
        //commentQuery.bindValue(":transaction_type", transactionType);
        //暂且这样获得评论
        commentQuery.bindValue(":transaction_type", "expense");
        if(!commentQuery.exec()){

            qDebug()<< "获取月评论错误：" << commentQuery.lastError();
        }

        if (commentQuery.next()) {
            comment = commentQuery.value("comment").toString();
        }
    }

    qDebug() << "top分类: " << topCategoryName;
    qDebug() << "总成交量: " << topCategoryTotal;
    qDebug() << "金额占比: " << maxPercentage << "%";
    qDebug() << "评论: " << comment;

    return comment;
}

// 按周筛选总金额占比最大的分类的评论
QString DatabaseManager::getTopCategoryByWeekWithComment(int year, int week, const QString &transactionType)
{
    // 查询某周的总金额
    QSqlQuery totalQuery;
    totalQuery.prepare(
        "SELECT SUM(amount) AS total_amount FROM bill_record "
        "WHERE year = :year "
        "AND week = :week "
        "AND transaction_type = :transaction_type;"
    );
    totalQuery.bindValue(":year", QString::number(year));
    totalQuery.bindValue(":week", QString::number(week).rightJustified(2, '0'));  // 保证周数是两位数
    totalQuery.bindValue(":transaction_type", transactionType);
    totalQuery.exec();

    double totalAmount = 0;
    if (totalQuery.next()) {
        totalAmount = totalQuery.value("total_amount").toDouble();
    }

    // 查询每个分类的金额总和
    QSqlQuery query;
    query.prepare(
        "SELECT c.name, COUNT(b.id) AS bill_count, SUM(b.amount) AS total_amount "
        "FROM bill_record b "
        "JOIN category c ON b.category_id = c.id "
        "WHERE year = :year "
        "AND week = :week "
        "AND b.transaction_type = :transaction_type "
        "GROUP BY c.name;"
    );
    query.bindValue(":year", QString::number(year));
    query.bindValue(":week", QString::number(week).rightJustified(2, '0'));
    query.bindValue(":transaction_type", transactionType);
    query.exec();

    // 查找占比最大的分类
    double maxPercentage = 0;
    QString topCategoryName;
    double topCategoryTotal = 0;
    while (query.next()) {
        QString categoryName = query.value("name").toString();
        double categoryTotal = query.value("total_amount").toDouble();

        // 计算占比
        double percentage = (totalAmount > 0) ? (categoryTotal / totalAmount) * 100 : 0;

        // 更新占比最大的分类
        if (percentage > maxPercentage) {
            maxPercentage = percentage;
            topCategoryName = categoryName;
            topCategoryTotal = categoryTotal;
        }
    }

    // 获取对应的评论
    QString comment;
    if (!topCategoryName.isEmpty()) {
        QSqlQuery commentQuery;

        commentQuery.prepare(
            "SELECT comment FROM comment "
            "WHERE category_id = (SELECT id FROM category WHERE name = :category_name "
            "AND type = :transaction_type);"
            );
        commentQuery.bindValue(":category_name", topCategoryName);
        //commentQuery.bindValue(":transaction_type", transactionType);
        //暂且这样获得评论
        commentQuery.bindValue(":transaction_type", "expense");
        if(!commentQuery.exec()){

            qDebug()<< "获取周评论错误：" << commentQuery.lastError();
        }

        if (commentQuery.next()) {
            comment = commentQuery.value("comment").toString();
        }
    }

    qDebug() << "top分类: " << topCategoryName;
    qDebug() << "总成交量: " << topCategoryTotal;
    qDebug() << "金额占比: " << maxPercentage << "%";
    qDebug() << "评论: " << comment;

    return comment;
}

// 修改某条消费记录
void DatabaseManager::updateRecord(int id, double amount, QString transaction_type, QString transactionDate, int categoryId, int methodId, QString counterparty, QString description, QString source_id, QString remark) {
    // 处理年/月/周
    QDateTime dt = QDateTime::fromString(transactionDate, "yyyy-MM-dd HH:mm:ss");
    int year = dt.date().year();
    int month = dt.date().month();

    int week;
    int weekYear;
    week = dt.date().weekNumber(&weekYear);

    if (methodId == 1) {       // 现金不需要 source_id
        source_id = "";
    }

    QSqlQuery query;
    query.prepare(
        "UPDATE bill_record SET "
        "transaction_date = :transaction_date, "
        "year = :year, "
        "month = :month, "
        "week = :week, "
        "amount = :amount, "
        "transaction_type = :transaction_type, "
        "category_id = :category_id, "
        "transaction_method_id = :method_id, "
        "counterparty = :counterparty, "
        "description = :description, "
        "source_id = :source_id, "
        "remark = :remark "
        "WHERE id = :id"
    );

    query.bindValue(":transaction_date", transactionDate);
    query.bindValue(":year", year);
    query.bindValue(":month", month);
    query.bindValue(":week", week);
    query.bindValue(":amount", amount);
    query.bindValue(":transaction_type", transaction_type);
    query.bindValue(":category_id", categoryId);
    query.bindValue(":method_id", methodId);
    query.bindValue(":counterparty", counterparty);
    query.bindValue(":description", description);
    query.bindValue(":source_id", source_id);
    query.bindValue(":remark", remark);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "修改记录失败: " << query.lastError();
    }
    else {
        qDebug() << "修改记录成功: ";
    }
}

// 新增一条消费记录
void DatabaseManager::addRecord(double amount, QString transaction_type, QString transactionDate, int categoryId, int methodId, QString counterparty, QString description, QString source_id, QString remark) {
    // 处理年/月/周
    QDateTime dt = QDateTime::fromString(transactionDate, "yyyy-MM-dd HH:mm:ss");
    int year = dt.date().year();
    int month = dt.date().month();

    int week;
    int weekYear;
    week = dt.date().weekNumber(&weekYear);

    QSqlQuery query;
    query.prepare(
            "INSERT INTO bill_record("
            "transaction_date, year, month, week, "
            "amount, transaction_type, "
            "category_id, transaction_method_id, "
            "counterparty, description, source_id, remark"
            ") VALUES ("
            ":transaction_date, :year, :month, :week, "
            ":amount, :transaction_type, "
            ":category_id, :method_id, "
            ":counterparty, :description, :source_id, :remark)"
        );

    if (methodId == 1) {
        source_id = "";
    }
    query.bindValue(":transaction_date", transactionDate);
    query.bindValue(":year", year);
    query.bindValue(":month", month);
    query.bindValue(":week", week);
    query.bindValue(":amount", amount);
    query.bindValue(":transaction_type", transaction_type);
    query.bindValue(":category_id", categoryId);
    query.bindValue(":method_id", methodId);
    query.bindValue(":counterparty", counterparty);
    query.bindValue(":description", description);
    query.bindValue(":source_id", source_id);
    query.bindValue(":remark", remark);

    if (!query.exec()) {
        qDebug() << "添加记录失败: " << query.lastError();
    }
    else {
        qDebug() << "添加记录成功: ";
    }
}

// 删除某条记录
void DatabaseManager::deleteRecord(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM bill_record WHERE id = :id");

    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "删除记录失败: " << query.lastError();
    }
    else {
        qDebug() << "删除记录成功: ";
    }
}

// 根据交易单号查询账单 ID
int DatabaseManager::getBillIdByTransactionNumber(QString sourceId) {
    QSqlQuery query;
    query.prepare("SELECT id FROM bill_record WHERE source_id = :source_id");

    query.bindValue(":source_id", sourceId);

    if (!query.exec()) {
        qDebug() << "Error fetching bill ID by transaction number: " << query.lastError();
        return -1; // 返回-1表示查询失败
    }

    if (query.next()) {
        return query.value(0).toInt(); // 返回账单 ID
    }

    return -1; // 如果没有找到结果，返回-1
}

// 根据交易时间查询消费订单ID
int DatabaseManager::getExpenseBillIdByDate(QString transactionDate) {
    int billId = -1;
    QSqlQuery query;
    query.prepare("SELECT id FROM bill_record WHERE transaction_date = :transaction_date AND transaction_type = 'expense'");

    query.bindValue(":transaction_date", transactionDate);

    if (!query.exec()) {
        qDebug() << "无法根据交易日期找到消费订单id: " << query.lastError();
        return billId;
    }

    query.next();
    billId = query.value(0).toInt();
    return billId;
}

// 根据交易时间查询收入订单ID
int DatabaseManager::getIncomeBillIdByDate(QString transactionDate) {
    int billId = -1;
    QSqlQuery query;
    query.prepare("SELECT id FROM bill_record WHERE transaction_date = :transaction_date AND transaction_type = 'income'");

    query.bindValue(":transaction_date", transactionDate);

    if (!query.exec()) {
        qDebug() << "无法根据日期找到收入订单id: " << query.lastError();
        return billId;
    }

    query.next();
    billId = query.value(0).toInt();
    return billId;
}
