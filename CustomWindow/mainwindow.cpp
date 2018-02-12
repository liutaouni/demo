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
    win->showMaximized();
}

void MainWindow::on_testDialogBtn_clicked()
{
    TestDialog *win = new TestDialog();
    win->showMaximized();
}
