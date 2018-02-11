#include "comwindow.h"
#include "ui_comwindow.h"

#include <QPainter>
#include <QMouseEvent>

#include <QDebug>

ComWindow::ComWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ComWindow)
{
    ui->setupUi(this);

    this->setStyleSheet("QWidget#ComWindow{border:1px solid #5284BC; background:#6BADF6;}");
    ui->contentWidget->setStyleSheet("QWidget#contentWidget{border:1px solid #5B93D1; background:#FFFFFF;}");

    this->setMouseTracking(true);
    ui->contentWidget->installEventFilter(this);
    ui->titleWidget->installEventFilter(this);
}

ComWindow::~ComWindow()
{
    delete ui;
}

bool ComWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(ui->contentWidget == watched)
    {
        if(event->type() == QEvent::Enter)
        {
            this->setCursor(Qt::ArrowCursor);
        }
    }
    else if(ui->titleWidget == watched)
    {
        if(event->type() == QEvent::Enter)
        {
            this->setCursor(Qt::ArrowCursor);
        }
    }

    return QWidget::eventFilter(watched, event);
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
    this->setStyleSheet("QWidget#ComWindow{border:1px solid #5284BC; background:#6BADF6;}");
    ui->contentWidget->setStyleSheet("QWidget#contentWidget{border:1px solid #5B93D1; background:#FFFFFF;}");
    this->repaint();
}

void ComWindow::showInactive()
{
    this->setStyleSheet("QWidget#ComWindow{border:1px solid #D3D3D3; background:#EBEBEB;}");
    ui->contentWidget->setStyleSheet("QWidget#contentWidget{border:1px solid #DADADA; background:#FFFFFF;}");
    this->repaint();
}

void ComWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        mIsMouseLeftBtnPressed = true;
    }

    QWidget::mousePressEvent(event);
}

void ComWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(mIsMouseLeftBtnPressed)
    {

    }
    else
    {
        updateCorsurStyleForDragBorder(event->pos());
    }

    QWidget::mouseMoveEvent(event);
}

void ComWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        mIsMouseLeftBtnPressed = false;
    }

    QWidget::mouseReleaseEvent(event);
}

void ComWindow::updateCorsurStyleForDragBorder(const QPoint &pos)
{
    int borderWidthLeft, borderWidthTop, borderWidthRight, borderWidthBottom;
    ui->winLayout->getContentsMargins(&borderWidthLeft, &borderWidthTop,
                                      &borderWidthRight, &borderWidthBottom);

    if(pos.x() < borderWidthLeft)
    {
        if(pos.y() < borderWidthTop)
        {
            this->setCursor(Qt::SizeFDiagCursor);
        }
        else if(pos.y() > this->height() - borderWidthBottom)
        {
            this->setCursor(Qt::SizeBDiagCursor);
        }
        else
        {
            this->setCursor(Qt::SizeHorCursor);
        }
    }
    else if(pos.x() > this->width() - borderWidthRight)
    {
        if(pos.y() < borderWidthTop)
        {
            this->setCursor(Qt::SizeBDiagCursor);
        }
        else if(pos.y() > this->height() - borderWidthBottom)
        {
            this->setCursor(Qt::SizeFDiagCursor);
        }
        else
        {
            this->setCursor(Qt::SizeHorCursor);
        }
    }
    else
    {
        if(pos.y() < borderWidthTop
                || pos.y() > this->height() - borderWidthBottom)
        {
            this->setCursor(Qt::SizeVerCursor);
        }
    }
}
