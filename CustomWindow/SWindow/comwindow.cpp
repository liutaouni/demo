#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

#include "comwindow.h"
#include "ui_comwindow.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDesktopWidget>

#include <QDebug>

ComWindow::ComWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ComWindow)
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

ComWindow::~ComWindow()
{
    delete ui;
}

bool ComWindow::eventFilter(QObject *watched, QEvent *event)
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

void ComWindow::updateWindowStyle(bool isActive)
{
    if(this->window()->windowState() & Qt::WindowMaximized)
    {
        if(isActive)
        {
            ui->winLayout->setContentsMargins(0, 0, 0, 0);
            ui->titleWidget->setFixedHeight(mMaxTitleHeight);
            ui->titleLayout->setContentsMargins(2, 0, 2, 0);
            this->setStyleSheet("QWidget#ComWindow{border:none; background:#6BADF6;}");
            ui->contentWidget->setStyleSheet("QWidget#contentWidget{border-top:1px solid #5B93D1; background:#FFFFFF;}");
        }
        else
        {
            ui->winLayout->setContentsMargins(0, 0, 0, 0);
            ui->titleWidget->setFixedHeight(mMaxTitleHeight);
            ui->titleLayout->setContentsMargins(2, 0, 2, 0);
            this->setStyleSheet("QWidget#ComWindow{border:none; background:#EBEBEB;}");
            ui->contentWidget->setStyleSheet("QWidget#contentWidget{border-top:1px solid #DADADA; background:#FFFFFF;}");
        }
    }
    else
    {
        if(isActive)
        {
            ui->winLayout->setContentsMargins(mBorderWidth, 1, mBorderWidth, mBorderWidth);
            ui->titleWidget->setFixedHeight(mNorTitleHeight);
            ui->titleLayout->setContentsMargins(0, 0, 0, 0);
            this->setStyleSheet("QWidget#ComWindow{border:1px solid #5284BC; background:#6BADF6;}");
            ui->contentWidget->setStyleSheet("QWidget#contentWidget{border:1px solid #5B93D1; background:#FFFFFF;}");
        }
        else
        {
            ui->winLayout->setContentsMargins(mBorderWidth, 1, mBorderWidth, mBorderWidth);
            ui->titleWidget->setFixedHeight(mNorTitleHeight);
            ui->titleLayout->setContentsMargins(0, 0, 0, 0);
            this->setStyleSheet("QWidget#ComWindow{border:1px solid #D3D3D3; background:#EBEBEB;}");
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

void ComWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        mIsLeftBtnPressed = true;
        mLeftBtnPressedPos = event->pos();
        mLeftBtnPressedGlobalPos = this->mapToGlobal(event->pos());
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
                    this->window()->showNormal();
                }
                else
                {
                    this->window()->move(mWinGeometry.topLeft() +
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

void ComWindow::mouseReleaseEvent(QMouseEvent *event)
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

void ComWindow::resizeEvent(QResizeEvent *event)
{
    updateWindowTitle(mWinTitle);

    QWidget::resizeEvent(event);
}

void ComWindow::updateCorsurStyleForDragBorder(const QPoint &pos)
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

void ComWindow::updateGeometryByDragBorder(const QPoint &pos)
{
    int xOffset = pos.x() - mLeftBtnPressedGlobalPos.x();
    int yOffset = pos.y() - mLeftBtnPressedGlobalPos.y();

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

void ComWindow::on_minBtn_clicked()
{
    this->window()->showMinimized();
}

void ComWindow::on_maxBtn_clicked()
{
    if(this->window()->windowState() & Qt::WindowMaximized){
        this->window()->showNormal();
    }else{
        this->window()->showMaximized();
    }
}

void ComWindow::on_closeBtn_clicked()
{
    this->window()->close();
}

void ComWindow::updateWindowTitle(const QString &title)
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

void ComWindow::updateWindowIcon(const QIcon &icon)
{
    ui->iconBtn->setIcon(icon);
}


