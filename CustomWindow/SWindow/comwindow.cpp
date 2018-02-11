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
            mDragRegion = EBorderNone;
        }
    }
    else if(ui->titleWidget == watched)
    {
        if(event->type() == QEvent::Enter)
        {
            this->setCursor(Qt::ArrowCursor);
            mDragRegion = EBorderNone;
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
        mIsLeftBtnPressed = true;
        mLeftBtnPressedPos = this->mapToGlobal(event->pos());
        mWinGeometry = this->window()->geometry();
    }

    QWidget::mousePressEvent(event);
}

void ComWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(mIsLeftBtnPressed)
    {
        if(mDragRegion != EBorderNone)
        {
            updateGeometryByDragBorder(mapToGlobal(event->pos()));
        }
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
        mIsLeftBtnPressed = false;
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
            mDragRegion = EBorderTopLeft;
        }
        else if(pos.y() > this->height() - borderWidthBottom)
        {
            this->setCursor(Qt::SizeBDiagCursor);
            mDragRegion = EBorderBottomLeft;
        }
        else
        {
            this->setCursor(Qt::SizeHorCursor);
            mDragRegion = EBorderLeft;
        }
    }
    else if(pos.x() > this->width() - borderWidthRight)
    {
        if(pos.y() < borderWidthTop)
        {
            this->setCursor(Qt::SizeBDiagCursor);
            mDragRegion = EBorderTopRight;
        }
        else if(pos.y() > this->height() - borderWidthBottom)
        {
            this->setCursor(Qt::SizeFDiagCursor);
            mDragRegion = EBorderBottomRight;
        }
        else
        {
            this->setCursor(Qt::SizeHorCursor);
            mDragRegion = EBorderRight;
        }
    }
    else
    {
        if(pos.y() < borderWidthTop)
        {
            this->setCursor(Qt::SizeVerCursor);
            mDragRegion = EBorderTop;
        }
        else if(pos.y() > this->height() - borderWidthBottom)
        {
            this->setCursor(Qt::SizeVerCursor);
            mDragRegion = EBorderBottom;
        }
    }
}

void ComWindow::updateGeometryByDragBorder(const QPoint &pos)
{
    int xOffset = pos.x() - mLeftBtnPressedPos.x();
    int yOffset = pos.y() - mLeftBtnPressedPos.y();

    QRect winRect = mWinGeometry;

    switch(mDragRegion)
    {
    case EBorderLeft:
        winRect.setLeft(winRect.left() + xOffset);
        if(winRect.width() < this->window()->minimumWidth())
        {
            winRect.setLeft(winRect.right() - this->window()->minimumWidth() + 1);
        }
        break;
    case EBorderTop:
        winRect.setTop(winRect.top() + yOffset);
        if(winRect.height() < this->window()->minimumHeight())
        {
            winRect.setTop(winRect.bottom() - this->window()->minimumHeight() + 1);
        }
        break;
    case EBorderRight:
        winRect.setRight(winRect.right() + xOffset);
        break;
    case EBorderBottom:
        winRect.setBottom(winRect.bottom() + yOffset);
        break;
    case EBorderTopLeft:
        winRect.setTop(winRect.top() + yOffset);
        winRect.setLeft(winRect.left() + xOffset);
        if(winRect.width() < this->window()->minimumWidth())
        {
            winRect.setLeft(winRect.right() - this->window()->minimumWidth() + 1);
        }
        if(winRect.height() < this->window()->minimumHeight())
        {
            winRect.setTop(winRect.bottom() - this->window()->minimumHeight() + 1);
        }
        break;
    case EBorderTopRight:
        winRect.setTop(winRect.top() + yOffset);
        winRect.setRight(winRect.right() + xOffset);
        if(winRect.height() < this->window()->minimumHeight())
        {
            winRect.setTop(winRect.bottom() - this->window()->minimumHeight() + 1);
        }
        break;
    case EBorderBottomLeft:
        winRect.setBottom(winRect.bottom() + yOffset);
        winRect.setLeft(winRect.left() + xOffset);
        if(winRect.width() < this->window()->minimumWidth())
        {
            winRect.setLeft(winRect.right() - this->window()->minimumWidth() + 1);
        }
        break;
    case EBorderBottomRight:
        winRect.setBottom(winRect.bottom() + yOffset);
        winRect.setRight(winRect.right() + xOffset);
        break;
    }

    this->window()->setGeometry(winRect);
}
