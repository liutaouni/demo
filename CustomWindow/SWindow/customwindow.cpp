#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

#include "customwindow.h"
#include "ui_customwindow.h"

#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QtMath>

CustomWindow::CustomWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomWindow)
{
    ui->setupUi(this);

    updateWindowStyle(false);

    this->setMouseTracking(true);

    ui->contentWidget->installEventFilter(this);
}

CustomWindow::~CustomWindow()
{
    delete ui;
}

bool CustomWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(ui->contentWidget == watched && event->type() == QEvent::Enter)
    {
        this->setCursor(Qt::ArrowCursor);
        mDragRegion = EBorderNone;
    }

    return QWidget::eventFilter(watched, event);
}

void CustomWindow::paintEvent(QPaintEvent *e)
{
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive( QStyle::PE_Widget, &o, &p, this);
    QWidget::paintEvent(e);

    if(!this->window()->isMaximized() && !this->window()->isFullScreen())
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing,true);

        QColor color(0,0,0,50);
        for(int i = 0 ; i < mBorderWidth ; ++i)
        {
            QPainterPath path;
            path.setFillRule(Qt::WindingFill);
            path.addRect(mBorderWidth-i,mBorderWidth-i,this->width()-(mBorderWidth-i)*2,this->height()-(mBorderWidth-i)*2);
            color.setAlpha(150 - qSqrt(i)*50);
            painter.setPen(color);
            painter.drawPath(path);
        }
    }
}

QWidget *CustomWindow::getContentWidget()
{
    return ui->contentWidget;
}

void CustomWindow::updateWindowStyle(bool)
{
    if(this->window()->windowState() & Qt::WindowMaximized)
    {
        ui->winLayout->setContentsMargins(0, 0, 0, 0);
        ui->contentWidget->setStyleSheet("QWidget#contentWidget{border:none; background:#FFFFFF;}");
    }
    else
    {
        ui->winLayout->setContentsMargins(mBorderWidth, mBorderWidth, mBorderWidth, mBorderWidth);
        ui->contentWidget->setStyleSheet("QWidget#contentWidget{border:1px solid #5B93D1; background:#FFFFFF;}");
    }

    this->repaint();
}

void CustomWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        mIsLeftBtnPressed = true;
        mLeftBtnPressedPos = event->pos();
        mLeftBtnPressedGlobalPos = this->mapToGlobal(event->pos());
        mWinGeometryPressed = this->window()->geometry();
    }

    QWidget::mousePressEvent(event);
}

void CustomWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(mIsLeftBtnPressed)
    {
        if(mDragRegion != EBorderNone)
        {
            QRect rect = qApp->desktop()->availableGeometry(this);
            QPoint pos = mapToGlobal(event->pos());
            if(rect.contains(pos))
            {
                updateGeometryByDragBorder(mapToGlobal(event->pos()));
            }
            else
            {
                if(pos.x() < rect.left()){
                    pos.setX(rect.left());
                }else if(pos.x() > rect.right()){
                    pos.setX(rect.right());
                }

                if(pos.y() < rect.top()){
                    pos.setY(rect.top());
                }else if(pos.y() > rect.bottom()){
                    pos.setY(rect.bottom());
                }

                QCursor::setPos(pos);
            }
        }
        else
        {
            QRect rect = qApp->desktop()->availableGeometry(this);
            QPoint pos = mapToGlobal(event->pos());

            if(rect.contains(pos))
            {
                if(this->window()->windowState() & Qt::WindowMaximized)
                {
                    int halfOfWinPre = mWinSizePre.width() / 2;
                    int halfOfWinCur = mWinSizeCur.width() / 2;

                    QPoint cursorPosOfWin;
                    if(mLeftBtnPressedPos.x() < halfOfWinCur)
                    {
                        if((mLeftBtnPressedPos.x() + mBorderWidth - 2) < halfOfWinPre){
                            cursorPosOfWin.setX(mLeftBtnPressedPos.x() + mBorderWidth - 2);
                        }else{
                            cursorPosOfWin.setX(halfOfWinPre);
                        }
                    }
                    else
                    {
                        if((mWinSizeCur.width() - mLeftBtnPressedPos.x() + mBorderWidth - 2) < halfOfWinPre){
                            cursorPosOfWin.setX(mWinSizePre.width() - (mWinSizeCur.width() - mLeftBtnPressedPos.x() + mBorderWidth - 2));
                        }else{
                            cursorPosOfWin.setX(halfOfWinPre);
                        }
                    }
                    cursorPosOfWin.setY(mLeftBtnPressedPos.y() + mBorderWidth + 1);

                    QPoint winPos = mLeftBtnPressedGlobalPos - cursorPosOfWin;

                    mWinGeometryPressed.setTopLeft(winPos);
                    mWinGeometryPressed.setRight(winPos.x() + mWinSizePre.width());
                    mWinGeometryPressed.setBottom(winPos.y() + mWinSizePre.height());

                    this->window()->showNormal();
                }
                else
                {
                    this->window()->move(mWinGeometryPressed.topLeft() +
                                         mapToGlobal(event->pos()) -
                                         mLeftBtnPressedGlobalPos);
                }
            }
            else
            {
                if(pos.x() < rect.left()){
                    pos.setX(rect.left());
                }else if(pos.x() > rect.right()){
                    pos.setX(rect.right());
                }

                if(pos.y() < rect.top()){
                    pos.setY(rect.top());
                }else if(pos.y() > rect.bottom()){
                    pos.setY(rect.bottom());
                }

                QCursor::setPos(pos);
            }
        }
    }
    else
    {
        if(this->window()->minimumSize() != this->window()->maximumSize()
                && !this->window()->isMaximized())
        {
            updateCorsurStyleForDragBorder(event->pos());
        }
    }

    QWidget::mouseMoveEvent(event);
}

void CustomWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        mIsLeftBtnPressed = false;

        QPoint pos = this->window()->pos();
        if(pos.y() < 0)
        {
            pos.setY(0);
            this->window()->move(pos);
        }
    }

    QWidget::mouseReleaseEvent(event);
}

void CustomWindow::resizeEvent(QResizeEvent *event)
{
    mWinSizePre = mWinSizeCur;
    mWinSizeCur = this->window()->size();

    QWidget::resizeEvent(event);
}

void CustomWindow::updateCorsurStyleForDragBorder(const QPoint &pos)
{
    if(pos.x() < mBorderWidth)
    {
        if(pos.y() < mBorderWidth)
        {
            this->setCursor(Qt::SizeFDiagCursor);
            mDragRegion = EBorderTopLeft;
        }
        else if(pos.y() > this->height() - mBorderWidth)
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
    else if(pos.x() > this->width() - mBorderWidth)
    {
        if(pos.y() < mBorderWidth)
        {
            this->setCursor(Qt::SizeBDiagCursor);
            mDragRegion = EBorderTopRight;
        }
        else if(pos.y() > this->height() - mBorderWidth)
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
        if(pos.y() < mBorderWidth)
        {
            this->setCursor(Qt::SizeVerCursor);
            mDragRegion = EBorderTop;
        }
        else if(pos.y() > this->height() - mBorderWidth)
        {
            this->setCursor(Qt::SizeVerCursor);
            mDragRegion = EBorderBottom;
        }
        else
        {
            this->setCursor(Qt::ArrowCursor);
            mDragRegion = EBorderNone;
        }
    }
}

void CustomWindow::updateGeometryByDragBorder(const QPoint &pos)
{
    int xOffset = pos.x() - mLeftBtnPressedGlobalPos.x();
    int yOffset = pos.y() - mLeftBtnPressedGlobalPos.y();

    QRect winRect = mWinGeometryPressed;

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

void CustomWindow::updateWindowTitle(const QString &)
{

}

void CustomWindow::updateWindowIcon(const QIcon &)
{

}


