#include "testwindow.h"
#include "ui_testwindow.h"

TestWindow::TestWindow(QWidget *parent) :
    SWindow(parent),
    ui(new Ui::TestWindow)
{
    ui->setupUi(SWindow::getContentWidget());
}

TestWindow::~TestWindow()
{
    delete ui;
}
