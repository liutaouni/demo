#include "fb_publicclass.h"
#include "screenshotwidgetmac.h"
#include "ui_toolwidget.h"
#include "fb_qdebug.h"
#include "staticmessagebox.h"
#include "fb_dir.h"

#include <QApplication>
#include <QPen>
#include <QBrush>
#include <QSize>
#include <QPainter>
#include <QRect>
#include <QFileDialog>
#include <QClipboard>
#include <QDateTime>
#include <QObject>
#include <QTextEdit>
#include <QTextBlock>
#include <QLabel>
#include <math.h>


#define BTN_ICON_SIZE_WIDTH 20
#define BTN_ICON_SIZE_HEIGHT 20
#define BTN_CHECK_STYLE  "QPushButton:hover {border-style: solid;  border-width:1px;  border-color: rgb(0,139,139);border-radius: 1px; background-color:rgb(173,216,230); }"\
"QPushButton:!hover { border-style: solid;  border-width:1px;  border-color: rgb(0,139,139);border-radius: 1px;background-color:transparent;}"
#define BTN_UNCHECK_STYLE  "QPushButton:hover {border-style: solid;  border-width:1px;  border-color: rgb(0,139,139);border-radius: 1px; background-color:rgb(173,216,230); }"\
"QPushButton:!hover { border:none;background-color:transparent;}"

directionBtn::directionBtn(QWidget *parent,directionStyle directS)
    :QPushButton(parent),m_directS(directS),m_pressed(false),m_startPos(QPoint(0, 0)), m_endPos(QPoint(0, 0)), m_changedPos(QPoint(0, 0))
{
   setBtnCursor();
   setStyleSheet("background: yellow; border: 0px");
}

directionBtn::~directionBtn()
{

}

void directionBtn::setBtnCursor()
{
    switch (m_directS) {
    case m_SizeLeft:
    case m_SizeRight:
            setCursor(Qt::SizeHorCursor);
        break;
    case m_SizeTop:
    case m_SizeBottom:
            setCursor(Qt::SizeVerCursor);
        break;
    case m_SizeLeftBottom:
    case m_SizeRightTop:
            setCursor(Qt::SizeBDiagCursor);
        break;
    case m_SizeLeftTop:
    case m_SizeRightBottom:
            setCursor(Qt::SizeFDiagCursor);
        break;
    default:
        break;
    }
}

void directionBtn::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        m_pressed = true;
        m_startPos = ev->pos();
    }
    ev->accept();
}

void directionBtn::mouseMoveEvent(QMouseEvent *ev)
{
    if(m_pressed)
    {
        m_endPos = ev->pos();
        m_changedPos = m_endPos-m_startPos;
        emit sig_directChangePos(m_directS, m_changedPos);
    }
    ev->accept();
}

void directionBtn::mouseReleaseEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton && m_pressed)
    {
        m_pressed = false;
        m_startPos = m_endPos = m_changedPos = QPoint(0, 0);
    }
    ev->accept();
}

selectRect::selectRect(QWidget *parent)
    :QLabel(parent),isMoving(false),isModified(false)
{
    setAutoFillBackground(true);
    setCursor(Qt::SizeAllCursor);

    this->setStyleSheet("QLabel {border:5px solid #6dcff6;}");

    topLeftBtn = new directionBtn(this, m_SizeLeftTop);
    topMidBtn = new directionBtn(this, m_SizeTop);
    topRightBtn = new directionBtn(this, m_SizeRightTop);

    midLeftBtn = new directionBtn(this, m_SizeLeft);
    midRighttBtn = new directionBtn(this, m_SizeRight);

    bottomLeftBtn = new directionBtn(this, m_SizeLeftBottom);
    bottomMidBtn = new directionBtn( this, m_SizeBottom);
    bottomRightBtn = new directionBtn(this, m_SizeRightBottom);

    setDirectionBtnEnabled(true);
    hide();
}

selectRect::~selectRect()
{

}

void selectRect::setDirectionBtnEnabled(bool mask)
{
    QObjectList list = this->children();
    foreach (QObject* obj, list) {
        directionBtn *btn = qobject_cast<directionBtn *>(obj);
        if(btn)
        {
            if(mask)
            {
                connect(btn, &directionBtn::sig_directChangePos, this, &selectRect::slot_directChangePos);
                btn->setBtnCursor();
            }
            else
            {
                disconnect(btn, &directionBtn::sig_directChangePos, this, &selectRect::slot_directChangePos);
                btn->setCursor(Qt::ArrowCursor);
            }
        }
    }
}

void selectRect::slotModified(bool mask)
{
    if(mask)
    {
        isModified = true;
        setCursor(Qt::ArrowCursor);
        setDirectionBtnEnabled(false);
    }
    else
    {
        isModified = false;
        setCursor(Qt::SizeAllCursor);
        setDirectionBtnEnabled(true);
    }
}

void selectRect::slot_directChangePos(directionStyle direct, QPoint pt)
{
//    QDEBUG() << "slot_directChangePos:" << pt;
    switch (direct) {
    case m_SizeLeft:
        if((m_rect.width()-pt.x()) > 20)
            m_rect.setLeft(m_rect.left()+pt.x());
        break;
    case m_SizeRight:
        if((m_rect.width()+pt.x()) > 20)
            m_rect.setRight(m_rect.right()+pt.x());
        break;
    case m_SizeTop:
        if((m_rect.height()-pt.y()) > 20)
            m_rect.setTop(m_rect.top()+pt.y());
        break;
    case m_SizeBottom:
        if((m_rect.height()+pt.y()) > 20)
            m_rect.setBottom(m_rect.bottom()+pt.y());
        break;
    case m_SizeLeftTop:
        if((m_rect.width()-pt.x()) > 20)
            m_rect.setLeft(m_rect.left()+pt.x());
        if((m_rect.height()-pt.y()) > 20)
            m_rect.setTop(m_rect.top()+pt.y());
        break;
    case m_SizeRightTop:
        if((m_rect.width()+pt.x()) > 20)
            m_rect.setRight(m_rect.right()+pt.x());
        if((m_rect.height()-pt.y()) > 20)
            m_rect.setTop(m_rect.top()+pt.y());
        break;
    case m_SizeLeftBottom:
        if((m_rect.width()-pt.x()) > 20)
            m_rect.setLeft(m_rect.left()+pt.x());
        if((m_rect.height()+pt.y()) > 20)
            m_rect.setBottom(m_rect.bottom()+pt.y());
        break;
    case m_SizeRightBottom:
        if((m_rect.width()+pt.x()) > 20)
            m_rect.setRight(m_rect.right()+pt.x());
        if((m_rect.height()+pt.y()) > 20)
            m_rect.setBottom(m_rect.bottom()+pt.y());
        break;
    default:
        break;
    }
    reFixWidget(m_rect);
    emit sigMovingRect(m_rect);

}

void selectRect::reFixWidget(QRect rect)
{
    m_rect = rect;
    setGeometry(rect);
    topLeftBtn->setGeometry(0, 0, 10, 10);
    topMidBtn->setGeometry(rect.width()/2-5, 0, 10, 10);
    topRightBtn->setGeometry(rect.width()-10, 0,  10, 10);

    midLeftBtn->setGeometry(0, rect.height()/2-5, 10, 10);
    midRighttBtn->setGeometry(rect.width()-10, rect.height()/2-5,  10, 10);

    bottomLeftBtn->setGeometry(0, rect.height()-10, 10, 10);
    bottomMidBtn->setGeometry(rect.width()/2-5, rect.height()-10, 10, 10);
    bottomRightBtn->setGeometry(rect.width()-10, rect.height()-10, 10, 10);
}

void selectRect::mousePressEvent(QMouseEvent *ev)
{
    if(isModified)
    {
        ev->ignore();
        return;
    }

    if(ev->button() == Qt::LeftButton)
    {
//        QDEBUG() << "selectRect mousePressEvent";
        isMoving = true;
        startPoint = ev->globalPos();
        ev->accept();
    }
}

void selectRect::mouseMoveEvent(QMouseEvent *ev)
{
    if(isModified)
    {
        ev->ignore();
        return;
    }

    if(isMoving)
    {
        endPoint =ev->globalPos();
        //m_rect.setX(m_rect.x() + endPoint.x() );
        m_rect.moveTo(m_rect.topLeft() + (endPoint-startPoint));
        startPoint = endPoint;
        //处理越界问题
        QRect rect = qApp->desktop()->availableGeometry();
        QPoint pt1 = m_rect.topLeft();

        if(pt1.x() <0)
            pt1.setX(0);
        else if(pt1.x()>(rect.right()-m_rect.width()))
            pt1.setX(rect.right()-m_rect.width());

        if(pt1.y() <0)
            pt1.setY(0);
        else if(pt1.y()>(rect.bottom()-m_rect.height()))
            pt1.setY(rect.bottom()-m_rect.height());

        m_rect.moveTo(pt1);

        reFixWidget(m_rect);
        emit sigMovingRect(m_rect);
        ev->accept();
    }
}

void selectRect::mouseReleaseEvent(QMouseEvent *ev)
{
    if(isModified)
    {
        ev->ignore();
        return;
    }

    if(ev->button() == Qt::LeftButton &&isMoving)
    {
        isMoving = false;
        ev->accept();
    }
}

ScreenshotWidget::ScreenshotWidget(QWidget *parent) :
    QLabel(parent),isDrawing(false),startPoint(QPoint(0,0)),endPoint(QPoint(0,0)),lastStartPoint(QPoint(0,0)), lastEndPoint(QPoint(0,0)),isModified(false),i(0),m_paintBrush(0),m_edit(NULL),isFirstPen(false)
{
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    setWindowModality(Qt::ApplicationModal);

    fullScreenPix =  QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId());

    //retina屏幕兼容处理
    int ratio = (int)QGuiApplication::primaryScreen()->devicePixelRatio();
    if(ratio == 2)
    {
        fullScreenPix = fullScreenPix.scaled(QGuiApplication::primaryScreen()->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    fullScreenTempPix = fullScreenPix;

    resize(fullScreenTempPix.width(), fullScreenTempPix.height());
    paint(fullScreenTempPix);

    selectWidget =new selectRect(this);
    connect(selectWidget, &selectRect::sigMovingRect, this, &ScreenshotWidget::slotMovingRect);
    connect(this, &ScreenshotWidget::sigModified, selectWidget, &selectRect::slotModified);
    setCursor(Qt::ArrowCursor);
    initToolWidget();
    m_pixmap = new QPixmap;
    positionStateLabel = new QLabel(this);
    show();
}

ScreenshotWidget::~ScreenshotWidget()
{
    delete ui;
    if(m_pixmap)
    {
        delete m_pixmap;
        m_pixmap = 0;
    }
    operateList.clear();
}

void ScreenshotWidget::paint(QPixmap &pixmap)
{
    QPainter painter(&pixmap);
    QBrush brush(QColor(25, 25,25, 127));
    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);
    painter.drawRect(QRect(QPoint(0,0), QPoint(width(), height())));

    painter.drawPixmap(shotRect, fullScreenPix, shotRect);
    update();
}

void ScreenshotWidget::mousePressEvent(QMouseEvent *ev)
{
    if(isModified && ev->button() == Qt::LeftButton)
    {
        startPoint = ev->pos();
        if(!shotRect.contains(startPoint))
            return;
        if(i==0)
        {
            if(m_content == p_Pen)
            {
                if(isFirstPen) //第一次如果为文字操作的话， 需要特殊处理，因为如果不处理的话，当输入完成后再点击的时候还会进入这里
                    return ;
                isFirstPen = true;
            }
            operateList.insert(0, fullScreenTempPix);
             QDEBUG() << "after insert,  operateList.count():" << operateList.count();
        }
        return;
    }
    if(ev->button() == Qt::LeftButton)
    {
        isDrawing = true;
        lastStartPoint = startPoint;
        startPoint = ev->pos();

        selectWidget->hide();
        positionStateLabel->hide();
        m_toolWidget->hide();
    }
    if(ev->button() == Qt::RightButton)
    {
        resetScreenShot();

        if(ev->type() == QEvent::MouseButtonDblClick)
        {
            slotCloseBtnClick();
        }
    }
    QLabel::mousePressEvent(ev);
}

void ScreenshotWidget::resetScreenShot()  //重新开始截图
{
    selectWidget->hide();
    positionStateLabel->hide();
    m_toolWidget->hide();

    operateList.clear();
    i = 0;
    isFirstPen = false;
    emit sigModified(false);

    fullScreenTempPix = fullScreenPix;
    shotRect = QRect();
    paint(fullScreenTempPix);
    isModified = false;

    ui->rectangleBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->ellipseBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->paintBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->arrowBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->texteditbtn->setStyleSheet(BTN_UNCHECK_STYLE);
}

void ScreenshotWidget::mouseMoveEvent(QMouseEvent *ev)
{
    if(isModified)
    {
        endPoint = ev->pos();
        if(!shotRect.contains(startPoint) || !shotRect.contains(endPoint)) //只能在高亮显示的图像内绘制
            return;
        if(i==0 && m_paintBrush==0)
        {
            fullScreenTempPix = fullScreenPix;
            paint(fullScreenTempPix);
        }
        else
        {
            fullScreenTempPix = tempPix;
            update();
        }

        QDEBUG() << "++++i:" << i;

        QPainter painter(&fullScreenTempPix);
        QPen pen;
        pen.setWidth(m_PenWidth);
        pen.setColor(m_PenColor);
        painter.setPen(pen);

        switch (m_content) {
        case p_Rect:
                painter.drawRect(QRect(startPoint, endPoint));
            break;
        case p_Ellipse:
                painter.drawEllipse(QRect(startPoint, endPoint));
            break;
        case p_Arrow:{
            float x1 = startPoint.x();
            float y1 = startPoint.y();

            float x2 = endPoint.x();
            float y2 = endPoint.y();

            float l = 10.0; //箭头的那长度
            float a = 0.5;  //箭头与线段角度

            float x3 = x2 - l * cos(atan2((y2 - y1), (x2 - x1)) - a);
            float y3 = y2 - l * sin(atan2((y2 - y1), (x2 - x1)) - a);

            float x4 = x2 - l * sin(atan2((x2 - x1), (y2 - y1)) - a);
            float y4 = y2 - l * cos(atan2((x2 - x1), (y2 - y1)) - a);

//            painter.drawLine(x1,y1,x3,y3);
//            painter.drawLine(x1,y1,x4,y4);
            painter.drawLine(x2,y2,x3,y3);
            painter.drawLine(x2,y2,x4,y4);
            painter.drawLine(startPoint,endPoint);
        }
            break;
        case p_Brush:
        {
            painter.drawLine(startPoint,endPoint);
            m_paintBrush++;
            if(m_paintBrush>0)
                startPoint = endPoint;

            tempPix = fullScreenTempPix;
        }
            break;
        case p_Pen:
        {

        }
            break;
        default:
            break;
        }
        update();
        return;
    }

    if(isDrawing)
    {
        fullScreenTempPix = fullScreenPix;
        endPoint = ev->pos();

        int minX = qMin(startPoint.x(), endPoint.x());
        int minY = qMin(startPoint.y(), endPoint.y());
        int maxX = qMax(startPoint.x(), endPoint.x());
        int maxY = qMax(startPoint.y(), endPoint.y());
        shotRect.setX(minX);
        shotRect.setY(minY);
        shotRect.setWidth(maxX-minX);
        shotRect.setHeight(maxY-minY);

        paint(fullScreenTempPix);
        showPositionStateWidget();
    }
    QLabel::mouseMoveEvent(ev);
}

void ScreenshotWidget::showPositionStateWidget()
{
    int x = (shotRect.x()+200)> width() ? (width()-200):shotRect.x();
    int y = (shotRect.y()-30) < 0? shotRect.y(): (shotRect.y()-30);
    positionStateLabel->setGeometry(x, y, 200, 30);
    positionStateLabel->setStyleSheet("background: #f0f0f0; color: blue;");
    positionStateLabel->setText(QString(" x: %1, y: %2, w: %3, h: %4 ").arg(QString::number(shotRect.x())).arg(QString::number(shotRect.y())).arg(QString::number(shotRect.width())).arg(QString::number(shotRect.height())));
    positionStateLabel->show();
}

void ScreenshotWidget::doWithTextEdit()
{
    if(!shotRect.contains(startPoint)) //只能在高亮显示的图像内绘制
        return;
    if(m_edit == NULL)
    {
        m_edit = new QTextEdit(this);
        m_edit->setObjectName("m_edit");
        this->m_edit->setStyleSheet("background: transparent; border: 1px solid #569DE5;");
        m_edit->setTextColor(m_PenColor);
        m_edit->setFontPointSize(m_Fontsize);
        connect(m_edit, &QTextEdit::textChanged, this, &ScreenshotWidget::slotInputEditTextChanged);
        m_edit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        //处理边界问题: 防止添加的控件超出可绘图区域
        QPoint temp(startPoint.x()+60, startPoint.y()+60);
        if(!shotRect.contains(temp))
        {
            if(temp.x()>shotRect.right())
                temp.setX(shotRect.right());
            if(temp.y()>shotRect.bottom())
                temp.setY(shotRect.bottom());

            m_edit->setGeometry(QRect(temp.x()-60, temp.y()-60, 60, 60));
        }
        else
            m_edit->setGeometry(QRect(startPoint, QSize(60, 60)));

        m_edit->show();
        m_edit->setFocus();
    }
    else
    {
        QPainter painter(&fullScreenTempPix);
        painter.setPen(QPen(m_PenColor));
        painter.setFont(QFont(qApp->font().family(), m_Fontsize));

        QString str = m_edit->textCursor().document()->toPlainText();
        painter.drawText(m_edit->geometry(),Qt::TextWrapAnywhere,str);
        update();
        m_edit->hide();
        delete m_edit;
        m_edit = NULL;

        *m_pixmap = fullScreenTempPix.copy(shotRect);
        tempPix = fullScreenTempPix;

        operateList.insert(0, fullScreenTempPix);
        QDEBUG() << "after insert,  operateList.count():" << operateList.count();
        i++;
    }
}

void ScreenshotWidget::slotInputEditTextChanged()
{
   int maxWidth = shotRect.right()-m_edit->x();
   int maxHeight = shotRect.bottom()-m_edit->y();

   while(maxLineCountOfBlock() > 1)
   {
       int width = this->m_edit->width() + 20;

       if(width <= maxWidth)
       {
           this->m_edit->setFixedWidth(width);
       }
       else
       {
           this->m_edit->setFixedWidth(maxWidth);
           break;
       }
   }

   int height = this->m_edit->document()->size().height()+10;
   if(height <= maxHeight)
   {
       this->m_edit->setFixedHeight(height);
   }
   else
   {
       this->m_edit->setFixedHeight(maxHeight);
   }
}

int ScreenshotWidget::maxLineCountOfBlock()
{
    int maxLineCount = 0;

    int blockCount = this->m_edit->document()->blockCount();
    for(int i = 0; i < blockCount; i++)
    {
        int lineCount = this->m_edit->document()->findBlockByNumber(i).layout()->lineCount();
        if(lineCount > maxLineCount)
        {
            maxLineCount = lineCount;
        }
    }

    return maxLineCount;
}

void ScreenshotWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    if(isModified && ev->button() == Qt::LeftButton)
    {
        endPoint = ev->pos();
//        QDEBUG() << "startPoint: " << startPoint << "shotRect.contains(startPoint):" << shotRect.contains(startPoint) << "endPoint: " << endPoint << "shotRect.contains(endPoint):" << shotRect.contains(endPoint);
        if(!shotRect.contains(startPoint) || !shotRect.contains(endPoint))
            return;

        if(m_content == p_Pen)
        {
            doWithTextEdit();
        }
        else
        {
            m_paintBrush = 0;
            *m_pixmap = fullScreenTempPix.copy(shotRect);
            tempPix = fullScreenTempPix;

            operateList.insert(0, fullScreenTempPix);
            QDEBUG() << "after insert,  operateList.count():" << operateList.count();
            i++;
        }
        return;
    }
    if(ev->button() == Qt::LeftButton)
    {
        isDrawing = false;
        lastEndPoint = endPoint;
        endPoint = ev->pos();
        if(startPoint == endPoint) //如果是同一个点，则本次操作无效
        {
            startPoint = lastEndPoint;
            endPoint = lastStartPoint;
        }

        selectWidget->reFixWidget(shotRect);
        selectWidget->show();
        positionStateLabel->show();
        showToolWidget(false);

        *m_pixmap = fullScreenTempPix.copy(shotRect);
    }
    QLabel::mouseReleaseEvent(ev);
}

void ScreenshotWidget::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape)
        slotCloseBtnClick();
    if(ev->modifiers() == Qt::ControlModifier && ev->key() == Qt::Key_Z)
        slotCancelBtnClick();
    if(ev->modifiers() == Qt::ControlModifier && ev->key() == Qt::Key_S)
        slotSaveBtnClick();

    return QLabel::keyPressEvent(ev);
}

void ScreenshotWidget::showEvent(QShowEvent *ev)
{
    showFullScreen();
//    showMaximized();
    setCursor(Qt::CrossCursor);
}

void ScreenshotWidget::closeEvent(QCloseEvent *ev)
{
    showNormal();
    QWidget::closeEvent(ev);
}

void ScreenshotWidget::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, fullScreenTempPix);

    QLabel::paintEvent(ev);
}

void ScreenshotWidget::slotMovingRect(QRect rect)
{
    shotRect = rect;

    fullScreenTempPix = fullScreenPix;
    paint(fullScreenTempPix);

    *m_pixmap = fullScreenTempPix.copy(shotRect);
    showToolWidget(false);
    showPositionStateWidget();
}


void ScreenshotWidget::initToolWidget()
{
    ui = new Ui::ToolWidget;
    m_toolWidget = new QWidget(this);
    m_toolWidget->setStyleSheet("QWidget {background: #f0f0f0; border:0px;} QPushButton:hover {border: 1px solid gray; }");

    ui->setupUi(m_toolWidget);
    ui->selectcolorlabel->setStyleSheet("QLabel#selectcolorlabel{border:1px solid rgb(110,126,149); background:rgb(255, 0, 0);}");

    m_toolWidget->setCursor(Qt::ArrowCursor);

    ui->saveButton->setIcon(QIcon(":/snapshotimages/save.png"));
    ui->saveButton->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->saveButton,SIGNAL(clicked()),this,SLOT(slotSaveBtnClick()));
    ui->saveButton->setToolTip("保存 Command+S");

    ui->cancelBtn->setIcon(QIcon(":/snapshotimages/cancel.png"));
    ui->cancelBtn->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->cancelBtn,SIGNAL(clicked()),this,SLOT(slotCancelBtnClick()));
    ui->cancelBtn->setToolTip("撤销编辑 Command+Z");

    ui->closeButton->setIcon(QIcon(":/snapshotimages/close.png"));
    ui->closeButton->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->closeButton,SIGNAL(clicked()),this,SLOT(slotCloseBtnClick()));

    ui->okButton->setIcon(QIcon(":/snapshotimages/ok.png"));
    ui->okButton->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->okButton,SIGNAL(clicked()),this,SLOT(slotOkbtnClick()));

    ui->rectangleBtn->setIcon(QIcon(":/snapshotimages/rect.png"));
    ui->rectangleBtn->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->rectangleBtn,SIGNAL(clicked()),this,SLOT(slotRectangleBtnClick()));

    ui->ellipseBtn->setIcon(QIcon(":/snapshotimages/ellips.png"));
    ui->ellipseBtn->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->ellipseBtn,SIGNAL(clicked()),this,SLOT(slotEllipseBtnClick()));

    ui->paintBtn->setIcon(QIcon(":/snapshotimages/brush.png"));
    ui->paintBtn->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->paintBtn,SIGNAL(clicked()),this,SLOT(slotPaintBtnClick()));

    ui->arrowBtn->setIcon(QIcon(":/snapshotimages/arrow.png"));
    ui->arrowBtn->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->arrowBtn,SIGNAL(clicked()),this,SLOT(slotArrowBtnClick()));

    ui->texteditbtn->setIcon(QIcon(":/snapshotimages/text.png"));
    ui->texteditbtn->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->texteditbtn,SIGNAL(clicked()),this,SLOT(slotTexteditBtnClick()));

    ui->swidthBtn->setIcon(QIcon(":/snapshotimages/small.png"));
    ui->swidthBtn->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->swidthBtn,SIGNAL(clicked()),this,SLOT(slotSmallWidthBtnClick()));

    ui->mwidthBtn->setIcon(QIcon(":/snapshotimages/middle.png"));
    ui->mwidthBtn->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->mwidthBtn,SIGNAL(clicked()),this,SLOT(slotMidWidthBtnClick()));

    ui->bwidthBtn->setIcon(QIcon(":/snapshotimages/large.png"));
    ui->bwidthBtn->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->bwidthBtn,SIGNAL(clicked()),this,SLOT(slotBigWidthBtnClick()));

    connect(ui->blackbtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->whitebtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->drakgraybtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->graybtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->drakredbtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->redbtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->drakyellowbtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->yellowbtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->drakgreenbtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->greenbtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->drakbluebtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->bluebtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->drakpinkbtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->pinkbtn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->color1btn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));
    connect(ui->color2btn,SIGNAL(clicked()),this,SLOT(slotColorlableClick()));

    m_PenWidth = 1;//设置默认值
    m_Fontsize = 12;
    m_PenColor = QColor(255, 0, 0);
    m_edit = NULL;
    slotSmallWidthBtnClick();
    m_toolWidget->hide();
}

void ScreenshotWidget::showToolWidget(bool mask)
{
    ui->settingwidget->setVisible(mask);
    m_toolWidget->adjustSize();
    m_toolWidget->resize(m_toolWidget->width()-45, m_toolWidget->height());

    int x = selectWidget->x();
    int y = selectWidget->y() + selectWidget->height();
    if ( (y + m_toolWidget->height()) > this->height())
    {
        y = y - m_toolWidget->height();
    }

    if(x+m_toolWidget->width()>this->width())
    {
        x = this->width() - m_toolWidget->width();
    }

    m_toolWidget->move(x,y);
    m_toolWidget->show();

}

void ScreenshotWidget::readyToPaint(paintContent content)
{
    m_content = content;
    emit sigModified(true);
    isModified = true;
    showToolWidget(true);
    if(m_edit)
        doWithTextEdit();
}

void ScreenshotWidget::slotRectangleBtnClick()
{  
    readyToPaint(p_Rect);
    ui->rectangleBtn->setStyleSheet(BTN_CHECK_STYLE);
    ui->ellipseBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->paintBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->arrowBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->texteditbtn->setStyleSheet(BTN_UNCHECK_STYLE);
}

void ScreenshotWidget::slotEllipseBtnClick()
{
    readyToPaint(p_Ellipse);
    ui->rectangleBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->ellipseBtn->setStyleSheet(BTN_CHECK_STYLE);
    ui->paintBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->arrowBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->texteditbtn->setStyleSheet(BTN_UNCHECK_STYLE);
}

void ScreenshotWidget::slotPaintBtnClick()
{
    readyToPaint(p_Brush);
    ui->rectangleBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->ellipseBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->paintBtn->setStyleSheet(BTN_CHECK_STYLE);
    ui->arrowBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->texteditbtn->setStyleSheet(BTN_UNCHECK_STYLE);
}

void ScreenshotWidget::slotArrowBtnClick()
{
    readyToPaint(p_Arrow);
    ui->rectangleBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->ellipseBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->paintBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->arrowBtn->setStyleSheet(BTN_CHECK_STYLE);
    ui->texteditbtn->setStyleSheet(BTN_UNCHECK_STYLE);
}

void ScreenshotWidget::slotTexteditBtnClick()
{
    readyToPaint(p_Pen);
    ui->rectangleBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->ellipseBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->paintBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->arrowBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->texteditbtn->setStyleSheet(BTN_CHECK_STYLE);
}

void ScreenshotWidget::updateTextEditState()
{
    if(this->m_edit != NULL)
    {
        QDEBUG()<<"setcolor entry";
        QTextCursor cursor = m_edit->textCursor();
        m_edit->selectAll();
        m_edit->setTextColor(this->m_PenColor);
        m_edit->setFontPointSize(m_Fontsize);
        m_edit->setFocus();

        m_edit->setTextCursor(cursor);
    }
}

void ScreenshotWidget::slotSmallWidthBtnClick()
{
    m_PenWidth = 1;
    m_Fontsize = 12;
    ui->swidthBtn->setStyleSheet(BTN_CHECK_STYLE);
    ui->mwidthBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->bwidthBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    updateTextEditState();
}

void ScreenshotWidget::slotMidWidthBtnClick()
{
    m_PenWidth = 3;
    m_Fontsize = 16;
    ui->swidthBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->mwidthBtn->setStyleSheet(BTN_CHECK_STYLE);
    ui->bwidthBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    updateTextEditState();
}

void ScreenshotWidget::slotBigWidthBtnClick()
{
    m_PenWidth = 7;
    m_Fontsize = 22;
    ui->swidthBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->mwidthBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->bwidthBtn->setStyleSheet(BTN_CHECK_STYLE);
    updateTextEditState();
}

void ScreenshotWidget::slotColorlableClick()
{
    QPushButton* qpbflag = qobject_cast<QPushButton*>(sender());
    QDEBUG() <<"@@slotColorlableClick "<< qpbflag->objectName();
    QDEBUG() <<"@@slotColorlableClick "<< qpbflag->styleSheet();

    QString colorstr = "background-color: rgb(255, 0, 0);";

    if(qpbflag->objectName() == "blackbtn")
    {
        this->m_PenColor = QColor(0,0,0);
        colorstr = "background-color: rgb(0, 0, 0);";
    }
    else if(qpbflag->objectName() == "whitebtn")
    {
        this->m_PenColor = QColor(255,255,255);
        colorstr = "background-color: rgb(255, 255, 255);";
    }
    else if(qpbflag->objectName() == "drakgraybtn")
    {
        this->m_PenColor = QColor(128,128,128);
        colorstr = "background-color: rgb(128, 128, 128);";
    }
    else if(qpbflag->objectName() == "graybtn")
    {
        this->m_PenColor = QColor(192,192,192);
        colorstr = "background-color: rgb(192, 192, 192);";
    }
    else if(qpbflag->objectName() == "drakredbtn")
    {
        this->m_PenColor = QColor(128,0,0);
        colorstr = "background-color: rgb(128, 0, 0);";
    }
    else if(qpbflag->objectName() == "redbtn")
    {
        this->m_PenColor = QColor(255,0,0);
        colorstr = "background-color: rgb(255, 0, 0);";
    }
    else if(qpbflag->objectName() == "drakyellowbtn")
    {
        this->m_PenColor = QColor(128,128,0);
        colorstr = "background-color: rgb(128, 128, 0);";
    }
    else if(qpbflag->objectName() == "yellowbtn")
    {
        this->m_PenColor = QColor(255,255,0);
        colorstr = "background-color: rgb(255, 255, 0);";
    }
    else if(qpbflag->objectName() == "drakgreenbtn")
    {
        this->m_PenColor = QColor(0,128,0);
        colorstr = "background-color: rgb(0, 128, 0);";
    }
    else if(qpbflag->objectName() == "greenbtn")
    {
        this->m_PenColor = QColor(0,255,0);
        colorstr = "background-color: rgb(0, 255, 0);";
    }
    else if(qpbflag->objectName() == "drakbluebtn")
    {
        this->m_PenColor = QColor(0,0,128);
        colorstr = "background-color: rgb(0, 0, 128);";
    }
    else if(qpbflag->objectName() == "bluebtn")
    {
        this->m_PenColor = QColor(0,0,255);
        colorstr = "background-color: rgb(0, 0, 255);";
    }else if(qpbflag->objectName() == "drakpinkbtn")
    {
        this->m_PenColor = QColor(128,0,128);
        colorstr = "background-color: rgb(128, 0, 128);";
    }
    else if(qpbflag->objectName() == "pinkbtn")
    {
        this->m_PenColor = QColor(255,0,255);
        colorstr = "background-color: rgb(255, 0, 255);";
    }
    else if(qpbflag->objectName() == "color1btn")
    {
        this->m_PenColor = QColor(0,128,128);
        colorstr = "background-color: rgb(0, 128, 128);";
    }
    else if(qpbflag->objectName() == "color2btn")
    {
        this->m_PenColor = QColor(0,255,255);
        colorstr = "background-color: rgb(0, 255, 255);";
    }

    QString colorStyle = "QLabel#selectcolorlabel{border-style: solid;  border-width:1px;  border-color: rgb(110,126,149);" + colorstr + "}";
    ui->selectcolorlabel->setStyleSheet(colorStyle);

    updateTextEditState();
}

void ScreenshotWidget::slotCancelBtnClick() //撤销操作
{
    if(operateList.count() >1)
    {
        operateList.removeAt(0);
        fullScreenTempPix = operateList.at(0);
        tempPix = fullScreenTempPix;
        update();
        *m_pixmap = fullScreenTempPix.copy(shotRect);
    }
    else
    {
        resetScreenShot();
    }
    QDEBUG() << "afer remove, operateList.count(): " << operateList.count();
}

void ScreenshotWidget::slotSaveBtnClick()
{
    if(m_edit)
        doWithTextEdit();

    isModified = false;
    QDateTime dateTime = QDateTime::currentDateTime();
    QString fileName = "屏幕截图" + dateTime.toString("yyyyMMddHHmmss") + ".png";
    QString filePath = QFileDialog::getSaveFileName(this, "保存图像", QDir::currentPath()+fileName, "PNG图像 (*.png)");
    if(!filePath.isEmpty())
    {
        m_pixmap->save(filePath,"PNG"); //保存成PNG图片
    }
    slotCloseBtnClick();
}

void ScreenshotWidget::slotCloseBtnClick()
{
    isModified = false;
    this->close();
    emit sigShotScreenFinished(false);
}

void ScreenshotWidget::slotOkbtnClick()
{
    StaticMessageBox::informationTimer(NULL, tr("已复制到剪贴板"), 2);

    if(m_edit)
        doWithTextEdit();

    isModified = false;
    WriteToClipboard(*m_pixmap);
    this->close();
    emit sigShotScreenFinished(true);
}

void ScreenshotWidget::WriteToClipboard(QPixmap pixmap)
{
    /*设置图像*/
    QApplication::clipboard()->setPixmap(pixmap);
}
