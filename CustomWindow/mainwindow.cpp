#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "testwindow.h"
#include "testdialog.h"

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

void MainWindow::on_testWindowBtn_clicked()
{
    TestWindow *win = new TestWindow();
    win->setWindowTitle("春天来了，天气凉了，一群大雁往南飞，一会儿");
    win->show();
}

void MainWindow::on_testDialogBtn_clicked()
{
    TestDialog *win = new TestDialog();
    win->show();
}
