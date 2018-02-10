#include "comwindow.h"
#include "ui_comwindow.h"

#include <QPainter>

ComWindow::ComWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ComWindow)
{
    ui->setupUi(this);

    setStyleSheet("QWidget#ComWindow{border:1px solid gray; border-radius:5px; background:darkGreen;}");
    ui->contentWidget->setStyleSheet("QWidget#contentWidget{border:1px solid gray; background:white;}");
}

ComWindow::~ComWindow()
{
    delete ui;
}

void ComWindow::paintEvent(QPaintEvent *e)
{
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive( QStyle::PE_Widget, &o, &p, this);
    QWidget::paintEvent(e);
}

QWidget *ComWindow::getContentWidget()
{
    return ui->contentWidget;
}
