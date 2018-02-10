#include "testdialog.h"
#include "ui_testdialog.h"

TestDialog::TestDialog(QWidget *parent) :
    SDialog(parent),
    ui(new Ui::TestDialog)
{
    ui->setupUi(SDialog::getContentWidget());
}

TestDialog::~TestDialog()
{
    delete ui;
}
