#include "comwindow.h"
#include "ui_comwindow.h"

#include <QPainter>

#include <QDebug>

ComWindow::ComWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ComWindow)
{
    ui->setupUi(this);

    setStyleSheet("QWidget#ComWindow{border:1px solid #5284BC; background:#6BADF6;}");
    ui->contentWidget->setStyleSheet("QWidget#contentWidget{border:1px solid #5B93D1; background:#FFFFFF;}");
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

void ComWindow::showActive()
{
    setStyleSheet("QWidget#ComWindow{border:1px solid #5284BC; background:#6BADF6;}");
    ui->contentWidget->setStyleSheet("QWidget#contentWidget{border:1px solid #5B93D1; background:#FFFFFF;}");
    this->repaint();
}

void ComWindow::showInactive()
{
    setStyleSheet("QWidget#ComWindow{border:1px solid #D3D3D3; background:#EBEBEB;}");
    ui->contentWidget->setStyleSheet("QWidget#contentWidget{border:1px solid #DADADA; background:#FFFFFF;}");
    this->repaint();
}
