#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "./src/db/database_manager.h"
#include "./src/ui/weekviewwidget.h"
#include "./src/ui/monthviewwidget.h"
#include "./src/ui/yearviewwidget.h"
#include "./src/ui/daydetailwidget.h"
#include "./src/ui/helpdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStyle>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentViewType("week")
    , currentTransactionType("支出")
{
    ui->setupUi(this);
    setupUI();

    // 数据库
    auto &db = DatabaseManager::instance();

    db.openDatabase();
    db.createTables();

    db.insertDefaultTables();

    // 检查数据库是否为空，决定显示空状态还是默认视图
    // TODO: 连接数据库检查逻辑
    // 暂时假设有数据，显示周度视图
    // showEmptyState();
    showWeekView();

    // 全局样式表
    qApp->setStyleSheet(
        // 按钮通用样式
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 5px; "
        "padding: 8px 16px; "
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
        "QPushButton:pressed { background-color: #2d5a8a; }"

        // 表格样式
        "QTableWidget { "
        "background-color: white; "
        "border: 1px solid #d0d8e0; "
        "gridline-color: #e0e8f0; "
        "}"
        "QTableWidget::item { "
        "padding: 5px; "
        "}"
        "QTableWidget::item:selected { "
        "background-color: #3b6ea5; "
        "color: white; "
        "}"

        // 下拉框样式
        "QComboBox { "
        "background-color: white; "
        "border: 1px solid #d0d8e0; "
        "border-radius: 5px; "
        "padding: 5px; "
        "}"
        "QComboBox:hover { border-color: #3b6ea5; }"
        "QComboBox::drop-down { "
        "border: none; "
        "width: 20px; "
        "}"

        // 输入框样式
        "QLineEdit { "
        "background-color: white; "
        "border: 1px solid #d0d8e0; "
        "border-radius: 5px; "
        "padding: 5px; "
        "}"
        "QLineEdit:focus { border-color: #3b6ea5; }"

        // 字体样式
        "* { "
           "  font-family: 'DengXian', 'Microsoft YaHei'; " // 优先使用等线，备选微软雅黑
           "  font-size: 13px; "
        "} "
    );
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setupUI()
{
    setWindowTitle("记账助手");
    setMinimumSize(600, 400);
    setStyleSheet("QMainWindow { background-color: #f0f4f8; }");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    setupTopBar();
    setupSideBar();
    setupContentArea();

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(topBar);

    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(0);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->addWidget(sideBar);
    bodyLayout->addWidget(contentStack, 1);

    mainLayout->addLayout(bodyLayout);
    mainLayout->setStretchFactor(bodyLayout, 1);

    qApp->setStyleSheet(qApp->styleSheet() +
        "QToolTip {"
        "  background-color: rgba(255, 255, 255, 230);" // 半透明白底
        "  border: 1px solid #3b6ea5;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  color: #333;"
        "  font-size: 12px;"
        "}"
    );
}

void MainWindow::setupTopBar()
{
    topBar = new QWidget(this);
    topBar->setFixedHeight(60);
    topBar->setStyleSheet(
        "QWidget { background-color: #6a8fc5; }"
        "QLabel { color: white; font-size: 18px; font-weight: bold; }"
    );

    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(20, 0, 20, 0);

    appTitleLabel = new QLabel("记账助手", topBar);
    topLayout->addWidget(appTitleLabel);

    topLayout->addStretch();

    helpButton = new QPushButton("?", topBar);
    helpButton->setFixedSize(35, 35);
    helpButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 15px; "
        "font-size: 20px; "
        "font-weight: bold; "
        "padding: 0;"
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
        "QPushButton:pressed { background-color: #2d5a8a; }"
    );
    connect(helpButton, &QPushButton::clicked, this, &MainWindow::onHelpClicked);
    topLayout->addWidget(helpButton);

    importButton = new QPushButton("导入", topBar);
    importButton->setFixedSize(80, 35);
    importButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 5px; "
        "font-size: 14px; "
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
        "QPushButton:pressed { background-color: #2d5a8a; }"
    );
    connect(importButton, &QPushButton::clicked, this, &MainWindow::onImportClicked);
    topLayout->addWidget(importButton);
    topLayout->addSpacing(10);
}

void MainWindow::setupSideBar()
{
    sideBar = new QWidget(this);
    sideBar->setFixedWidth(150);
    sideBar->setStyleSheet(
    "QWidget { background-color: #DAE3F3; font-family: 'Microsoft YaHei UI', 'SimHei', 'Arial'; }"
           "QPushButton { "
           "background-color: transparent; "
           "color:  #666; "
           "text-align: left; "
           "padding: 15px 20px; "
           "border: none; "
           "font-size: 16px; "
           "}"
           "QPushButton:hover { "
           "background-color: transparent; "
           "color: #1e3a5f; "
           "}"
           "QPushButton:checked { "
           "color: white; "
           "}"

    );

    QVBoxLayout *sideLayout = new QVBoxLayout(sideBar);
    sideLayout->setSpacing(0);
    sideLayout->setContentsMargins(0, 20, 0, 20);

    weekButton = new QPushButton("周度", sideBar);
    weekButton->setCheckable(true);
    connect(weekButton, &QPushButton::clicked, this, &MainWindow::onWeekViewClicked);
    sideLayout->addWidget(weekButton);

    monthButton = new QPushButton("月度", sideBar);
    monthButton->setCheckable(true);
    connect(monthButton, &QPushButton::clicked, this, &MainWindow::onMonthViewClicked);
    sideLayout->addWidget(monthButton);

    yearButton = new QPushButton("年度", sideBar);
    yearButton->setCheckable(true);
    connect(yearButton, &QPushButton::clicked, this, &MainWindow::onYearViewClicked);
    sideLayout->addWidget(yearButton);

    sideLayout->addStretch();
}

void MainWindow::setupContentArea()
{
    contentStack = new QStackedWidget(this);

    // 空状态页面
    emptyStateWidget = new QWidget();
    QVBoxLayout *emptyLayout = new QVBoxLayout(emptyStateWidget);
    emptyLayout->setAlignment(Qt::AlignCenter);

    QPushButton *startImportBtn = new QPushButton("开始导入");
    startImportBtn->setFixedSize(150, 50);
    startImportBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #3b6ea5; "
        "color: white; "
        "border: none; "
        "border-radius: 5px; "
        "font-size: 16px; "
        "}"
        "QPushButton:hover { background-color: #4a7fb8; }"
    );
    connect(startImportBtn, &QPushButton::clicked, this, &MainWindow::onImportClicked);
    emptyLayout->addWidget(startImportBtn);

    QHBoxLayout *hintLayout = new QHBoxLayout();
    QLabel *arrowLabel = new QLabel("→");
    arrowLabel->setStyleSheet("color: #3b6ea5; font-size: 20px;");
    QLabel *hintLabel = new QLabel("点击问号查看教程");
    hintLabel->setStyleSheet("color: #666; font-size: 14px;");
    hintLayout->addStretch();
    hintLayout->addWidget(arrowLabel);
    hintLayout->addWidget(hintLabel);
    hintLayout->addStretch();
    emptyLayout->addLayout(hintLayout);

    // 各视图页面
    weekViewWidget = new WeekViewWidget();
    monthViewWidget = new MonthViewWidget();
    yearViewWidget = new YearViewWidget();
    dayDetailWidget = new DayDetailWidget();

    connect(dayDetailWidget, SIGNAL(backToMainView()), this, SLOT(onBackFromDetail()));

       // 连接周度视图的日期点击信号
    connect(weekViewWidget, SIGNAL(dayClicked(QString)), this, SLOT(showDayDetailView(QString)));

       // 连接月度视图的日期点击信号
    connect(monthViewWidget, SIGNAL(dayClicked(QString)), this, SLOT(showDayDetailView(QString)));

    contentStack->addWidget(emptyStateWidget);
    contentStack->addWidget(weekViewWidget);
    contentStack->addWidget(monthViewWidget);
    contentStack->addWidget(yearViewWidget);
    contentStack->addWidget(dayDetailWidget);
}

void MainWindow::showEmptyState()
{
    contentStack->setCurrentWidget(emptyStateWidget);
}

void MainWindow::showWeekView()
{
    currentViewType = "week";
    contentStack->setCurrentWidget(weekViewWidget);
    weekButton->setChecked(true);
    weekButton->setStyleSheet(
        "QPushButton { "
        "background-color: #1e3a5f; "
        "color: white; "
        "}"
    );
    monthButton->setChecked(false);
    yearButton->setChecked(false);
    monthButton->setStyleSheet("");
    yearButton->setStyleSheet("");
}

void MainWindow::showMonthView()
{
    currentViewType = "month";
    contentStack->setCurrentWidget(monthViewWidget);
    monthButton->setChecked(true);
    monthButton->setStyleSheet(
        "QPushButton { "
        "background-color: #1e3a5f; "
        "color: white; "
        "}"
    );
    weekButton->setChecked(false);
    yearButton->setChecked(false);
    weekButton->setStyleSheet("");
    yearButton->setStyleSheet("");
}

void MainWindow::showYearView()
{
    currentViewType = "year";
    contentStack->setCurrentWidget(yearViewWidget);
    yearButton->setChecked(true);
    yearButton->setStyleSheet(
        "QPushButton { "
        "background-color: #1e3a5f; "
        "color: white; "
        "}"
    );
    weekButton->setChecked(false);
    monthButton->setChecked(false);
    weekButton->setStyleSheet("");
    monthButton->setStyleSheet("");
}

void MainWindow::showDayDetailView(const QString &date)
{
    contentStack->setCurrentWidget(dayDetailWidget);
    dayDetailWidget->loadDayData(date);
}

void MainWindow::onImportClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择支付宝账单文件",
        "",
        "CSV Files (*.csv);;All Files (*)"
    );

    if (!filePath.isEmpty()) {
        // TODO: 将文件路径传给后端
        // QJsonObject request;
        // request["source"] = "alipay";
        // request["filePath"] = filePath;
        // emit importRequested(request);

        QMessageBox::information(this, "导入", "文件路径: " + filePath);

        // 导入后切换到周度视图
        showWeekView();
    }
}

void MainWindow::onHelpClicked()
{
    HelpDialog *helpDialog = new HelpDialog(this);
    helpDialog->exec();
}

void MainWindow::onWeekViewClicked()
{
    showWeekView();
}

void MainWindow::onMonthViewClicked()
{
    showMonthView();
}

void MainWindow::onYearViewClicked()
{
    showYearView();
}

void MainWindow::onBackFromDetail()
{
    if (currentViewType == "week") {
        showWeekView();
    } else if (currentViewType == "month") {
        showMonthView();
    } else if (currentViewType == "year") {
        showYearView();
    }
}

