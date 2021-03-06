﻿#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

#include "comwindow_win.h"
#include "ui_comwindow_win.h"

#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopWidget>

ComWindow_Win::ComWindow_Win(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ComWindow_Win)
{
    ui->setupUi(this);

    updateWindowStyle(false);
    ui->minBtn->setStyleSheet("QPushButton#minBtn{border-image:url(:/icon/icon/min.png)}"
                              "QPushButton#minBtn:hover{border-image:url(:/icon/icon/min_hover.png)}");

    this->setMouseTracking(true);
    ui->titleLabel->setMouseTracking(true);
    ui->titleWidget->setMouseTracking(true);

    ui->iconBtn->installEventFilter(this);
    ui->btnsWidget->installEventFilter(this);
    ui->contentWidget->installEventFilter(this);
    ui->titleWidget->installEventFilter(this);
}

ComWindow_Win::~ComWindow_Win()
{
    delete ui;
}

bool ComWindow_Win::eventFilter(QObject *watched, QEvent *event)
{
    if((ui->contentWidget == watched && event->type() == QEvent::Enter)
            || (ui->iconBtn == watched && event->type() == QEvent::Enter)
            || (ui->btnsWidget == watched && event->type() == QEvent::Enter))
    {
        if(event->type() == QEvent::Enter)
        {
            this->setCursor(Qt::ArrowCursor);
            mDragRegion = EBorderNone;
        }
    }
    else if(ui->titleWidget == watched && event->type() == QEvent::MouseButtonDblClick)
    {
        on_maxBtn_clicked();

        return true;
    }

    return QWidget::eventFilter(watched, event);
}

void ComWindow_Win::paintEvent(QPaintEvent *e)
{
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive( QStyle::PE_Widget, &o, &p, this);
    QWidget::paintEvent(e);
}

QWidget *ComWindow_Win::getContentWidget()
{
    return ui->contentWidget;
}

void ComWindow_Win::updateWindowStyle(bool isActive)
{
    if(this->window()->windowState() & Qt::WindowMaximized)
    {
        if(isActive)
        {
            ui->winLayout->setContentsMargins(0, 0, 0, 0);
            ui->titleWidget->setFixedHeight(mMaxTitleHeight);
            ui->titleLayout->setContentsMargins(2, 0, 2, 0);
            this->setStyleSheet("QWidget#ComWindow_Win{border:none; background:#6BADF6;}");
            ui->contentWidget->setStyleSheet("QWidget#contentWidget{border-top:1px solid #5B93D1; background:#FFFFFF;}");
        }
        else
        {
            ui->winLayout->setContentsMargins(0, 0, 0, 0);
            ui->titleWidget->setFixedHeight(mMaxTitleHeight);
            ui->titleLayout->setContentsMargins(2, 0, 2, 0);
            this->setStyleSheet("QWidget#ComWindow_Win{border:none; background:#EBEBEB;}");
            ui->contentWidget->setStyleSheet("QWidget#contentWidget{border-top:1px solid #DADADA; background:#FFFFFF;}");
        }
    }
    else
    {
        if(isActive)
        {
            ui->winLayout->setContentsMargins(mBorderWidth, mBorderTopWidth, mBorderWidth, mBorderWidth);
            ui->titleWidget->setFixedHeight(mNorTitleHeight);
            ui->titleLayout->setContentsMargins(0, 0, 0, 0);
            this->setStyleSheet("QWidget#ComWindow_Win{border:1px solid #5284BC; background:#6BADF6;}");
            ui->contentWidget->setStyleSheet("QWidget#contentWidget{border:1px solid #5B93D1; background:#FFFFFF;}");
        }
        else
        {
            ui->winLayout->setContentsMargins(mBorderWidth, mBorderTopWidth, mBorderWidth, mBorderWidth);
            ui->titleWidget->setFixedHeight(mNorTitleHeight);
            ui->titleLayout->setContentsMargins(0, 0, 0, 0);
            this->setStyleSheet("QWidget#ComWindow_Win{border:1px solid #D3D3D3; background:#EBEBEB;}");
            ui->contentWidget->setStyleSheet("QWidget#contentWidget{border:1px solid #DADADA; background:#FFFFFF;}");
        }
    }

    if(isActive){
        ui->closeBtn->setStyleSheet("QPushButton#closeBtn{border-image:url(:/icon/icon/close_active.png)}"
                                    "QPushButton#closeBtn:hover{border-image:url(:/icon/icon/close_hover.png)}");
    }else{
        ui->closeBtn->setStyleSheet("QPushButton#closeBtn{border-image:url(:/icon/icon/close_deactive.png)}"
                                    "QPushButton#closeBtn:hover{border-image:url(:/icon/icon/close_hover.png)}");
    }

    if(this->window()->windowState() & Qt::WindowMaximized){
        ui->maxBtn->setStyleSheet("QPushButton#maxBtn{border-image:url(:/icon/icon/restore.png)}"
                                  "QPushButton#maxBtn:hover{border-image:url(:/icon/icon/restore_hover.png)}");
    }else{
        ui->maxBtn->setStyleSheet("QPushButton#maxBtn{border-image:url(:/icon/icon/max.png)}"
                                  "QPushButton#maxBtn:hover{border-image:url(:/icon/icon/max_hover.png)}");
    }

    this->repaint();
}

void ComWindow_Win::mousePressEvent(QMouseEvent *event)
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

void ComWindow_Win::mouseMoveEvent(QMouseEvent *event)
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
        else if(mLeftBtnPressedPos.y() < ui->titleWidget->height())
        {
            QRect rect = qApp->desktop()->availableGeometry(this);
            QPoint pos = mapToGlobal(event->pos());

            if(rect.contains(pos))
            {
                if(this->window()->windowState() & Qt::WindowMaximized)
                {
                    int halfOfWinPre = (mWinSizePre.width() - (ui->btnsWidget->width() - ui->iconBtn->width()))/2;
                    int halfOfWinCur = (mWinSizeCur.width() - (ui->btnsWidget->width() - ui->iconBtn->width()))/2;

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
                    cursorPosOfWin.setY(mLeftBtnPressedPos.y()+(mNorTitleHeight-mMaxTitleHeight-2));

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

void ComWindow_Win::mouseReleaseEvent(QMouseEvent *event)
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

void ComWindow_Win::resizeEvent(QResizeEvent *event)
{
    updateWindowTitle(mWinTitle);

    mWinSizePre = mWinSizeCur;
    mWinSizeCur = this->window()->size();

    QWidget::resizeEvent(event);
}

void ComWindow_Win::updateCorsurStyleForDragBorder(const QPoint &pos)
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

void ComWindow_Win::updateGeometryByDragBorder(const QPoint &pos)
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

void ComWindow_Win::on_minBtn_clicked()
{
    this->window()->showMinimized();
}

void ComWindow_Win::on_maxBtn_clicked()
{
    if(this->window()->windowState() & Qt::WindowMaximized){
        this->window()->showNormal();
    }else{
        this->window()->showMaximized();
    }
}

void ComWindow_Win::on_closeBtn_clicked()
{
    this->window()->close();
}

void ComWindow_Win::updateWindowTitle(const QString &title)
{
    mWinTitle = title;
    QString disTitle = mWinTitle;

    QFontMetrics fm(ui->titleLabel->font());
    if(fm.width(disTitle) > ui->titleLabel->width()){
        ui->titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        disTitle = fm.elidedText(disTitle, Qt::ElideRight, qMax(ui->titleLabel->width(), ui->titleLabel->minimumWidth()*2));
        disTitle.replace("…", "...");
    }else{
        ui->titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    }

    ui->titleLabel->setText(disTitle);
}

void ComWindow_Win::updateWindowIcon(const QIcon &icon)
{
    ui->iconBtn->setIcon(icon);
}


