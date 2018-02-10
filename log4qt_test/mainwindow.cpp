#include "mainwindow.h"
#include "ui_mainwindow.h"


#include "log4qt/logmanager.h"
#include "log4qt/logger.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    Log4Qt::LogManager::rootLogger()->info("button clicked");
}
