#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager::DatabaseManager()
{
}

DatabaseManager::~DatabaseManager()
{
    if (db.isOpen())
        db.close();
}

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

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
        " source_id TEXT,"
        " FOREIGN KEY(category_id) REFERENCES category(id),"
        " FOREIGN KEY(transaction_method_id) REFERENCES transaction_method(id)"
        ");";

    if (!query.exec(billSql)) {
        qDebug() << "创建 bill_record 失败:" << query.lastError().text();
        return false;
    }

    return true;
}

void DatabaseManager::insertDefaultCategories()
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

    QSqlQuery q;
    int id = 1;

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
}

bool DatabaseManager::isReady() const
{
    return ready;
}
