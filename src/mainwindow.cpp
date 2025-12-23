#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "./src/db/database_manager.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 数据库
    auto &db = DatabaseManager::instance();

    db.openDatabase();
    db.createTables();
    db.insertDefaultTables();

    db.importAlipayCsv(
        QStringLiteral("D:/softwareProject/test/支付宝交易明细(20250922-20251222).csv")
    );


}

MainWindow::~MainWindow()
{
    delete ui;
}

