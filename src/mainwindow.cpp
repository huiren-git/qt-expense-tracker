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
    db.insertDefaultCategories();
}

MainWindow::~MainWindow()
{
    delete ui;
}

