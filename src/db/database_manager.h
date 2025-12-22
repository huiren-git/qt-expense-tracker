#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QSqlDatabase>

class DatabaseManager
{
public:
    static DatabaseManager& instance();

    bool openDatabase();
    bool createTables();
    void insertDefaultCategories();
    bool isReady() const;

private:
    DatabaseManager();
    ~DatabaseManager();

    QSqlDatabase db;
    bool ready = false;
};

#endif // DATABASE_MANAGER_H
