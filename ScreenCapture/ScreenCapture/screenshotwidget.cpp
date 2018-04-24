#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif


#include "screenshotwidget.h"
#include "ui_toolwidget.h"
#include <QFileDialog>
#include <QDateTime>
#include "global.h"
#include <QDesktopWidget>
#include <QTimer>
#include <QPainter>
#include <QBuffer>
#include <QtCore>
#include <QClipboard>
#include <QBitmap>
#include <QScreen>
#include <QTextBlock>

#pragma comment(lib, "user32")

#define BTN_ICON_SIZE_WIDTH 20
#define BTN_ICON_SIZE_HEIGHT 20
#define BTN_CHECK_STYLE  "QPushButton:hover {border-style: solid;  border-width:1px;  border-color: rgb(0,139,139);border-radius: 1px; background-color:rgb(173,216,230); }"\
"QPushButton:!hover { border-style: solid;  border-width:1px;  border-color: rgb(0,139,139);border-radius: 1px;background-color:transparent;}"
#define BTN_UNCHECK_STYLE  "QPushButton:hover {border-style: solid;  border-width:1px;  border-color: rgb(0,139,139);border-radius: 1px; background-color:rgb(173,216,230); }"\
"QPushButton:!hover { border:none;background-color:transparent;}"


ScreenshotWidget::ScreenshotWidget(QWidget *parent) :m_autoRectTimer(NULL),
    QLabel(parent)
{
    QString qssFile = ":/snapshotqss/screenshot.qss";
    QFile file(qssFile);
    file.open(QFile::ReadOnly);
    QString style = QLatin1String(file.readAll());
    file.close();

    this->setStyleSheet(style);

    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);  //使窗口置顶
    setWindowModality(Qt::ApplicationModal);

    /*显示阴影的label*/
    m_label = new QLabel(this);
    m_label->resize(size());
    //QDEBUG() << "ScreenShotWidget: " << size() << "m_label:" << m_label->size();
    m_label->show();

    /*显示高亮图像的部分*/
    m_pixLabel = new QLabel(this);
    m_pixLabel->setObjectName("pixlabel");
    m_pixLabel->hide();
    #ifdef SHOT_SCREEN_AUTO_IDENTIFY_WIN
    /*更新自动显示的区域的定时器*/
    m_autoRectTimer = new QTimer(this);
    m_autoRectTimer->setInterval(500);
	connect(m_autoRectTimer,SIGNAL(timeout()),this,SLOT(slotUpdateCusorWindowInfo()));
    #endif

    /*选择范围*/
    m_selectRect = new SelectRect;
    m_selectRect->setParent(this);
    connect(m_selectRect,SIGNAL(sigRectChanged(QRect)),this,SLOT(slotRectChanged(QRect)));
    connect(m_selectRect,SIGNAL(sigSelectrectFinished(QRect)),this,SLOT(slotRectSelectFinished(QRect)));
    connect(m_selectRect,SIGNAL(sigReleaseWithoutMove()),this,SLOT(slotAutoPixSelect()));
    connect(m_selectRect,SIGNAL(sigPaint(bool)),this,SLOT(paint(bool)));


    m_infolabel = new QLabel(this);
    m_infolabel->setObjectName("infopixlabel");
    m_infolabel->hide();
    m_infolabel->setCursor(Qt::ArrowCursor);
//    QPainter infoP(infoPix);
//    infoP.setBrush(QBrush(QColor(Qt::green),Qt::SolidPattern));
//    infoP.drawRect(0,0,200,32);

    initToolWidget();

    hide();

    m_State = FINISHED;    
}

ScreenshotWidget::~ScreenshotWidget()
{
    QList<QPixmap*>::iterator it;
    for(it = m_operStack.begin(); it != m_operStack.end(); ++it)
        delete *it;
    m_operStack.clear();

    delete m_infolabel;
    delete m_selectRect;
    delete m_pixLabel;
    if(m_autoRectTimer)
    {
        m_autoRectTimer->stop();
        delete m_autoRectTimer;
    }
    delete m_label;
    delete m_toolWidget;
    delete ui;
}

void ScreenshotWidget::initToolWidget()
{
    ui = new Ui::ToolWidget;
    m_toolWidget = new QWidget(this);
    ui->setupUi(m_toolWidget);
    m_toolWidget->setCursor(Qt::ArrowCursor);
    m_toolWidget->hide();

    ui->saveButton->setIcon(QIcon(":/snapshotimages/save.png"));
    ui->saveButton->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->saveButton,SIGNAL(clicked()),this,SLOT(slotSaveBtnClick()));

    ui->cancelBtn->setIcon(QIcon(":/snapshotimages/cancel.png"));
    ui->cancelBtn->setIconSize(QSize(BTN_ICON_SIZE_WIDTH,BTN_ICON_SIZE_HEIGHT));
    connect(ui->cancelBtn,SIGNAL(clicked()),this,SLOT(slotCancelBtnClick()));

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

    this->m_PenWidth = 1;//设置默认值
    ui->swidthBtn->setChecked(true);
    this->m_Fontsize = 12;
    this->m_PenColor = Qt::red;
    this->m_edit = NULL;
}

void ScreenshotWidget::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (event->type() == QEvent::MouseButtonDblClick)
        {
            if (m_State == SHOTTING)
            {
                slotOkbtnClick();
            }
        }
    }
    else if (event->button() == Qt::RightButton)
    {
        if (m_State == READY)
        {
            finishShot();
            emit sigShotScreenFinished(false);
        }
        else
        {
            finishShot();
            getReadyToShot();
            this->show();
        }
    }
}

/****************************************
*
*按键功能
*
****************************************/
void ScreenshotWidget::keyReleaseEvent(QKeyEvent *e)
{
    // esc 退出
    if( e->key()==Qt::Key_Escape )
    {
        //this->close();
        slotCloseBtnClick();
    }
    // s 保存截取区域
    else if( e->key()==Qt::Key_S && e->modifiers() == Qt::ControlModifier)
    {
        //saveScreen();
        slotSaveBtnClick();
        //this->close();
    }
    // s 保存截取区域
    else if( e->key()==Qt::Key_Z && e->modifiers() == Qt::ControlModifier)
    {
        slotCancelBtnClick();
    }
}

void ScreenshotWidget::getReadyToShot()
{
    // 释放
    QList<QPixmap*>::iterator it;
    for(it = m_operStack.begin(); it != m_operStack.end(); ++it)
        delete *it;
    m_operStack.clear();

    m_autoRect.setTopLeft(QPoint(-100,-100));
    m_autoRect.setSize(QSize(0,0));
    m_rect.setTopLeft(QPoint(-100,-100));
    m_rect.setSize(QSize(0,0));

	int w = 0, h = 0;

	for(int i=0; i< qApp->desktop()->screenCount(); i++)
	{
		w += qApp->desktop()->screenGeometry(i).width();
		h = qMax(h, qApp->desktop()->screenGeometry(i).height());
	}

	QPixmap shotPix = qApp->primaryScreen()->grabWindow(0, 0, 0, w, h);

    //retina屏幕兼容处理
    int ratio = (int)QGuiApplication::primaryScreen()->devicePixelRatio();
    if(ratio == 2)
    {
		//shotPix = shotPix.scaled(QGuiApplication::primaryScreen()->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    m_fullScreenPixmap = new QPixmap;
    *m_fullScreenPixmap = shotPix.copy();
    this->setPixmap(*m_fullScreenPixmap);

	m_selectRect->resize(w, h);
	m_label->resize(w, h);
	QPixmap pix = QPixmap(w, h);
    pix.fill(QColor(25,25,25,127));  //用蓝色填充这个pixmap  且透明度为127 (255表示不透明)
    m_label->setPixmap(pix);
    m_label->show();

    m_pixmap = new QPixmap;
    m_oldCursorPos = QPoint(-100,-100);
    m_selectRect->getReadyToSelect();

    //截图信息显示区域 背景
    m_infoPix = new QPixmap(150,15);

    m_State = READY;
	//showFullScreen();
	this->resize(w, h);

    setCursor(Qt::CrossCursor);

    #ifdef SHOT_SCREEN_AUTO_IDENTIFY_WIN
    slotUpdateCusorWindowInfo();
    if(m_autoRectTimer)
    {
        m_autoRectTimer->start();
    }
    #endif

    ui->settingwidget->setVisible(false);
    m_toolWidget->adjustSize();
}

void ScreenshotWidget::finishShot()
{
    m_State = FINISHED;
    m_pixLabel->clear(); //显示高亮图像的label
    m_pixLabel->hide();
    this->clear();
    this->hide();
    m_label->clear();
    m_label->hide();
    m_toolWidget->hide();

    m_infolabel->clear();
    m_infolabel->hide();

    if(m_infoPix!=NULL)
    {
        delete m_infoPix;
        m_infoPix = NULL;
    }

    if(m_fullScreenPixmap != NULL)
    {
        delete m_fullScreenPixmap;
        m_fullScreenPixmap = NULL;
    }
    if(m_pixmap != NULL)
    {
        delete m_pixmap;
        m_pixmap = NULL;
    }


    m_selectRect->clear();
    if(this->m_edit!=NULL)
    {
        this->m_edit->clear();
        delete this->m_edit;
        this->m_edit = NULL;
    }
    if(m_autoRectTimer)
    {
        m_autoRectTimer->stop();
	}
}

void ScreenshotWidget::updateAutoPix()
{
    if (m_State != READY) return;
    int x = m_autoRect.x();
    int y = m_autoRect.y();
    int borderWidth = 2; //边框的宽度
    int width = m_autoRect.width();
    int height= m_autoRect.height();
    QPixmap autoPix = m_fullScreenPixmap->copy(x,y,width,height);
    QSize totalSize = QSize(width + borderWidth*2,height + borderWidth *2);
    QPixmap pix  = QPixmap(totalSize);
    pix.fill(Qt::cyan);
    QPainter painter(&pix);
    painter.drawPixmap(borderWidth,borderWidth,autoPix);

    if (x <= 0) //区域超出左边
    {
        painter.setPen(QPen(Qt::cyan,borderWidth*2,Qt::SolidLine,Qt::RoundCap));
        painter.drawLine(QPoint(borderWidth,0),QPoint(borderWidth,height));
    }
    if (y <= 0) //区域超出上边
    {
        painter.setPen(QPen(Qt::cyan,borderWidth*2,Qt::SolidLine,Qt::RoundCap));
        painter.drawLine(QPoint(0,borderWidth),QPoint(width,borderWidth));
    }
    if ((x + width) >= m_fullScreenPixmap->width()) //区域超出右边
    {
        painter.setPen(QPen(Qt::cyan,borderWidth*2,Qt::SolidLine,Qt::RoundCap));
        painter.drawLine(QPoint(width+borderWidth,0),QPoint(width+borderWidth,height));
    }
    if ((y + height) >= m_fullScreenPixmap->height()) //区域超出下边
    {
        painter.setPen(QPen(Qt::cyan,borderWidth*2,Qt::SolidLine,Qt::RoundCap));
        painter.drawLine(QPoint(0,height+borderWidth),QPoint(width,height+borderWidth));
    }

    m_pixLabel->setPixmap(pix);
    m_pixLabel->resize(totalSize);
    m_pixLabel->move(x-borderWidth,y-borderWidth);
	m_pixLabel->show();
}

void ScreenshotWidget::updatePixmap()
{
    int x = m_rect.left();
    int y = m_rect.top();
    int width = m_rect.width();
    int height= m_rect.height();

    QPixmap pix = QPixmap(size());
    pix.fill(QColor(25,25,25,127));  //用蓝色填充这个pixmap  且透明度为127 (255表示不透明)
    m_label->setPixmap(pix);
    m_label->show();

    *m_pixmap = m_fullScreenPixmap->copy(x,y,width,height);
//    QPen pen;
//    pen.setColor(Qt::red);
//    pen.setWidth(1);
//    pen.setStyle(Qt::SolidLine);
//    painter.setPen(pen);

    //显示截取区域信息 width height
    QPainter painter(m_infoPix);
    QPen pen = QPen(Qt::white, 1, Qt::SolidLine);
    painter.setPen(pen);
    painter.setBrush(QBrush(QColor(Qt::gray),Qt::SolidPattern));
    painter.drawRect(0,0,150,15);
    painter.drawText(0,10,QString(" X:%1 Y:%2 W:%3 H:%4").arg(x).arg(y).arg(width).arg(height));
    m_infolabel->setPixmap(*m_infoPix);
    if(y-15>0)
    {
        m_infolabel->move(x,y-15);
    }
    else
    {
        m_infolabel->move(x,y);
    }

    m_infolabel->show();


    m_pixLabel->setPixmap(*m_pixmap);
    m_pixLabel->resize(m_rect.size());
    m_pixLabel->move(m_rect.topLeft());
    m_pixLabel->show();
    showToolWidget();
}

void ScreenshotWidget::showToolWidget()
{
    int x = m_rect.x();
    int y = m_rect.y() + m_rect.height();
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

void ScreenshotWidget::slotAutoPixSelect()
{
    //QDEBUG()<<"slotAutoPixSelect";
    m_State = SHOTTING;
    if(m_autoRectTimer)
    {
        m_autoRectTimer->stop();
    }

    m_rect = m_autoRect;
    updatePixmap();
    m_selectRect->setRect(m_rect);
    //加入操作队列中
    QPixmap* opitem = new QPixmap();
    *opitem = m_fullScreenPixmap->copy();
    this->m_operStack.append(opitem);
}

void ScreenshotWidget::slotRectChanged(QRect re)
{
    m_State = SHOTTING;
    if(m_autoRectTimer)
    {
        m_autoRectTimer->stop();
    }

    m_rect = re;
    //QDEBUG()<<"slotrectChanged"<< QDateTime::currentDateTime().toString("hh:mm:ss:zzz");
    updatePixmap();
}

void ScreenshotWidget::slotRectSelectFinished(QRect)
{
    //加入操作队列中
    QPixmap* opitem = new QPixmap();
    *opitem = m_fullScreenPixmap->copy();
    this->m_operStack.append(opitem);
}

void ScreenshotWidget::slotSaveBtnClick()
{
    inputTextEditFocusout();
    QDateTime dateTime = QDateTime::currentDateTime();//获取系统现在的时间
    QString fileName = "屏幕截图" + dateTime.toString("yyyyMMddhhmmss") + ".png"; //设置显示格式

    QString filePath = QFileDialog::getSaveFileName(
        this, "保存图像",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + fileName,//初始目录
         "PNG图像 (*.png)");
    if (!filePath.isEmpty())
    {
        m_pixmap->save(filePath,"PNG"); //保存成PNG图片
        finishShot();
        emit sigShotScreenFinished(false);
    }
}

void ScreenshotWidget::slotCancelBtnClick()
{
    //QDEBUG()<<"slotCancelBtnClick";
    if(this->m_operStack.isEmpty())
    {
        //重新准备截屏
        finishShot();
        getReadyToShot();
        this->show();
    }
    else
    {
        //QDEBUG()<<"slotCancelBtnClick mid count=" << this->m_operStack.count();
        //撤销显示
        //先删除末尾
        delete this->m_operStack.takeLast();

        if(this->m_operStack.isEmpty())
        {
            //重新准备截屏
            finishShot();
            getReadyToShot();
            this->show();
        }
        else
        {
            //然后弹出末尾值
            QPixmap tempfullScreenPixmap = this->m_operStack.last()->copy();

            //QDEBUG()<<"slotCancelBtnClick mid 1";
            setPixmap(tempfullScreenPixmap);
            drawEditUpdatePixmap(tempfullScreenPixmap);
            //QDEBUG()<<"slotCancelBtnClick mid 2";
            *m_fullScreenPixmap = tempfullScreenPixmap.copy();
            //QDEBUG()<<"slotCancelBtnClick mid 3";
        }

        //QDEBUG()<<"slotCancelBtnClick out";
    }
}

void ScreenshotWidget::slotCloseBtnClick()
{
    finishShot();
    emit sigShotScreenFinished(false);
}

void ScreenshotWidget::slotOkbtnClick()
{
    inputTextEditFocusout();
    hide();

    ScreenShotSaveClipboard(*m_pixmap);
    finishShot();
    emit sigShotScreenFinished(false);
}

void ScreenshotWidget::slotRectangleBtnClick()
{
    //QDEBUG()<<"slotRectangleBtnClick";
    inputTextEditFocusout();
    showPenSettingPanel(this->m_curShape,Rectangle);
    this->m_curShape = Rectangle;
    m_selectRect->setDrawEditModel(true);
    ui->rectangleBtn->setStyleSheet(BTN_CHECK_STYLE);
    ui->ellipseBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->paintBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->arrowBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->texteditbtn->setStyleSheet(BTN_UNCHECK_STYLE);
}

void ScreenshotWidget::slotEllipseBtnClick()
{
    //QDEBUG()<<"slotEllipseBtnClick";
    inputTextEditFocusout();
    showPenSettingPanel(this->m_curShape,Ellipse);
    this->m_curShape = Ellipse;
    m_selectRect->setDrawEditModel(true);
    ui->rectangleBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->ellipseBtn->setStyleSheet(BTN_CHECK_STYLE);
    ui->paintBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->arrowBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->texteditbtn->setStyleSheet(BTN_UNCHECK_STYLE);
}

void ScreenshotWidget::slotPaintBtnClick()
{
    //QDEBUG()<<"slotPaintBtnClick";
    inputTextEditFocusout();
    showPenSettingPanel(this->m_curShape,Painter);
    this->m_curShape = Painter;
    m_selectRect->setDrawEditModel(true);
    ui->rectangleBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->ellipseBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->paintBtn->setStyleSheet(BTN_CHECK_STYLE);
    ui->arrowBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->texteditbtn->setStyleSheet(BTN_UNCHECK_STYLE);
}

void ScreenshotWidget::slotArrowBtnClick()
{
    //QDEBUG()<<"slotArrowBtnClick";
    inputTextEditFocusout();
    showPenSettingPanel(this->m_curShape,Arrow);
    this->m_curShape = Arrow;
    m_selectRect->setDrawEditModel(true);
    ui->rectangleBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->ellipseBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->paintBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->arrowBtn->setStyleSheet(BTN_CHECK_STYLE);
    ui->texteditbtn->setStyleSheet(BTN_UNCHECK_STYLE);
}

void ScreenshotWidget::slotTexteditBtnClick()
{
    //QDEBUG()<<"slotTexteditBtnClick";
    showPenSettingPanel(this->m_curShape,TextInput);
    this->m_curShape = TextInput;
    m_selectRect->setDrawEditModel(true);
    ui->rectangleBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->ellipseBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->paintBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->arrowBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->texteditbtn->setStyleSheet(BTN_CHECK_STYLE);
}

void ScreenshotWidget::slotSmallWidthBtnClick()
{
    this->m_PenWidth = 1;
    this->m_Fontsize = 12;
    //选中样式设置
    setTextInputEditFontSize();
    ui->bwidthBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->swidthBtn->setStyleSheet(BTN_CHECK_STYLE);
    ui->mwidthBtn->setStyleSheet(BTN_UNCHECK_STYLE);
}

void ScreenshotWidget::slotMidWidthBtnClick()
{
    this->m_PenWidth = 3;
    this->m_Fontsize = 16;
    setTextInputEditFontSize();
    ui->bwidthBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->swidthBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->mwidthBtn->setStyleSheet(BTN_CHECK_STYLE);
}

void ScreenshotWidget::slotBigWidthBtnClick()
{
    this->m_PenWidth = 7;
    this->m_Fontsize = 22;
    setTextInputEditFontSize();
    ui->bwidthBtn->setStyleSheet(BTN_CHECK_STYLE);
    ui->swidthBtn->setStyleSheet(BTN_UNCHECK_STYLE);
    ui->mwidthBtn->setStyleSheet(BTN_UNCHECK_STYLE);
}

void ScreenshotWidget::slotInputEditTextChange()
{
    int maxWidth = m_pixLabel->x()+m_pixLabel->width()-this->m_edit->x();
    int maxHeight = m_pixLabel->y()+m_pixLabel->height()-this->m_edit->y();

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

void ScreenshotWidget::slotColorlableClick()
{
    QPushButton* qpbflag = qobject_cast<QPushButton*>(sender());
    //QDEBUG() <<"@@slotColorlableClick "<< qpbflag->objectName();
    //QDEBUG() <<"@@slotColorlableClick "<< qpbflag->styleSheet();
    //qpbflag->style();
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

    if(this->m_edit != NULL)
    {
        //QDEBUG()<<"setcolor entry";
        QTextCursor cursor = this->m_edit->textCursor();
        this->m_edit->selectAll();
        this->m_edit->setTextColor(this->m_PenColor);
        this->m_edit->setFocus();

        this->m_edit->setTextCursor(cursor);
    }
}
#ifdef SHOT_SCREEN_AUTO_IDENTIFY_WIN
QRect ScreenshotWidget::getCursorWindowInfo()
{
#ifdef Q_OS_WIN
    int x = 0;
    int y = 0;
    int width = 0;
    int height= 0;

    POINT curPoint; //鼠标坐标
    GetCursorPos(&curPoint); //获取鼠标的坐标
	setWindowOpacity(0);
    //QDEBUG()<<"@@@getCursorWindowInfo ##"<< QDateTime::currentDateTime().toString("hh:mm:ss:zzz");;
    HWND mainWindow = WindowFromPoint(curPoint); //获取鼠标所在点的窗口句柄
    //QDEBUG()<<"@@@getCursorWindowInfo ####"<< QDateTime::currentDateTime().toString("hh:mm:ss:zzz");;
	setWindowOpacity(1);

    RECT mainWindowRect; //主窗口的位置信息
    //mainWindowRect.left=mainWindowRect.right=mainWindowRect.bottom=mainWindowRect.top = 0;
    if (GetWindowRect(mainWindow,&mainWindowRect))
    {
        /*主窗口的客户区*/
        RECT mainwindowClientRect;  //窗口的客户区(即是不包含标题栏和状态栏的部分)
        GetClientRect(mainWindow,&mainwindowClientRect);

        /*主窗口客服区的起点*/
        POINT mcPoint;
        mcPoint.x = 0; //这里的初始化不能能少,否则会出错
        mcPoint.y = 0;
        ClientToScreen(mainWindow,&mcPoint);

        x = mcPoint.x;
        y = mcPoint.y;
        width = mainwindowClientRect.right - mainwindowClientRect.left;
        height= mainwindowClientRect.bottom - mainwindowClientRect.top;
    }
    if (curPoint.x<x||curPoint.x>(x+width)||curPoint.y<y||curPoint.y>(y+height))
    {
        x = mainWindowRect.left;
        y = mainWindowRect.top;
        width = mainWindowRect.right - mainWindowRect.left;
        height= mainWindowRect.bottom- mainWindowRect.top;
    }
    if (x < 0) //区域超出左边
    {
        width = width + x;
        x = 0;
    }
    if (y < 0) //区域超出上边
    {
        height = height + y;
        y = 0;
    }
    if ((x + width) > m_fullScreenPixmap->width()) //区域超出右边
    {
        width = m_fullScreenPixmap->width() - x;
    }
    if ((y + height) > m_fullScreenPixmap->height()) //区域超出下边
    {
        height = m_fullScreenPixmap->height() - y;
    }

    QRect rect;
    rect.setX(x);
    rect.setY(y);
    rect.setWidth(width);
    rect.setHeight(height);

    //QDEBUG()<<"@@@getCursorWindowInfo rect="<<rect<< QDateTime::currentDateTime().toString("hh:mm:ss:zzz");
    return rect;
#else
    return QRect();
#endif

}
#endif

void ScreenshotWidget::drawEditUpdatePixmap(QPixmap editpixmap)
{
    int x = m_rect.left();
    int y = m_rect.top();
    int width = m_rect.width();
    int height= m_rect.height();
    *m_pixmap = editpixmap.copy(x,y,width,height);
    m_pixLabel->setPixmap(*m_pixmap);
}

void ScreenshotWidget::showPenSettingPanel(ShapeType oldtype,ShapeType newtype)
{
    if(oldtype != newtype)
    {
        ui->settingwidget->setVisible(true);
        m_toolWidget->adjustSize();
        showToolWidget();
        return;
    }

    if(ui->settingwidget->isVisible())
    {
        ui->settingwidget->setVisible(false);        
    }
    else
    {
        ui->settingwidget->setVisible(true);
    }
    m_toolWidget->adjustSize();
    showToolWidget();
}

void ScreenshotWidget::inputTextEditFocusout()
{
    if(this->m_edit != NULL)
    {
        //QDEBUG()<< "@@@ focusout:";
        this->m_rc = QRect(this->m_edit->geometry().x()+4,this->m_edit->geometry().y()+7,
                           this->m_edit->document()->size().width(),this->m_edit->document()->size().height());
        this->m_inputText = this->m_edit->toPlainText();
        paint(m_fullScreenPixmap);
        delete this->m_edit;
        this->m_edit = NULL;
    }
}

void ScreenshotWidget::setTextInputEditFontSize()
{
    if(this->m_edit != NULL)
    {
        //QDEBUG()<<"setTextInputEditFontSize entry";
        QTextCursor cursor = this->m_edit->textCursor();
        this->m_edit->selectAll();
        this->m_edit->setFontPointSize(this->m_Fontsize);
        this->m_edit->setFocus();

        this->m_edit->setTextCursor(cursor);
    }
}

void ScreenshotWidget::ScreenShotSaveClipboard(QPixmap pix)
{   
    WriteToClipboard(pix);

    emit sigShotScreenFinished(true);
}

void ScreenshotWidget::WriteToClipboard(QPixmap pixmap)
{
    /*设置图像*/
    QApplication::clipboard()->setPixmap(pixmap);
}
#ifdef SHOT_SCREEN_AUTO_IDENTIFY_WIN
void ScreenshotWidget::slotUpdateCusorWindowInfo()
{
    if (m_State != READY) return;

    if (m_oldCursorPos != QCursor::pos())
    {
        //timer->stop();
        m_oldCursorPos = QCursor::pos();
        QRect re = getCursorWindowInfo();
        if(re != m_autoRect)
        {//此次的区域与原来的不一样才进行更新
            m_autoRect = re;
            updateAutoPix();
        }
        //timer->start();
    }

}
#endif

void ScreenshotWidget::paint(bool isend)
{
    //QDEBUG()<<"paint";
    if(this->m_curShape == None || this->m_curShape == Painter)
    {
        paint(m_fullScreenPixmap,isend);
    }
    else if(this->m_curShape == TextInput )
    {
        if(isend)
        {
            if(this->m_edit==NULL)
            {
                this->m_edit = new QTextEdit(this);
                this->m_edit->setObjectName("m_edit");
                this->m_edit->setStyleSheet("background: transparent;");
                this->m_edit->resize(QSize(50,50));
                this->m_edit->move(m_selectRect->getLastPoint());//你喜欢的位置。
                this->m_edit->show();
                this->m_edit->setFocus();
                connect(this->m_edit,SIGNAL(textChanged()),this,SLOT(slotInputEditTextChange()));
                QSizePolicy localSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                this->m_edit->setSizePolicy(localSizePolicy);
                this->m_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                this->m_edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                this->m_edit->setFontPointSize(this->m_Fontsize);
                this->m_edit->setTextColor(this->m_PenColor);

            }
            else
            {
                //QDEBUG()<< "@@@ focusout:";
                //this->m_rc = this->m_edit->geometry();
                this->m_rc = QRect(this->m_edit->geometry().x()+4,this->m_edit->geometry().y()+7,
                                   this->m_edit->document()->size().width(),this->m_edit->document()->size().height());
                this->m_inputText = this->m_edit->toPlainText();
                paint(m_fullScreenPixmap,true);
                delete this->m_edit;
                this->m_edit = NULL;
            }
        }

    }
    else
    {
        if(isend)
        {
            paint(m_fullScreenPixmap,true);
        }
        else
        {
            QPixmap tempfullScreenPixmap = m_fullScreenPixmap->copy();
            paint(&tempfullScreenPixmap,false);
        }
    }
}

void ScreenshotWidget::paint(QPixmap *fullpixmap,bool isrecord)
{
    int scale =1;

    QPainter pp(fullpixmap);
    QPen pen = QPen(this->m_PenColor, this->m_PenWidth, Qt::SolidLine, Qt::RoundCap);
    pp.setPen(pen);

    int x,y,w,h;
    QPoint lastPoint = m_selectRect->getLastPoint();
    QPoint endPoint = m_selectRect->getEndPoint();
    x = lastPoint.x()/scale;
    y = lastPoint.y()/scale;
    w = endPoint.x()/scale - x;
    h = endPoint.y()/scale - y;
    //QDEBUG()<<"paint x="<<x<<"y="<<y<<" w="<<w<<" h="<<h;
    switch(this->m_curShape)
    {
    case None:
    case Painter:
    {
        pp.drawLine(lastPoint/scale,endPoint/scale);
        //lastPoint = endPoint;
        m_selectRect->setLastPoint(endPoint);
        break;
    }
    case Line:
    {
        pp.drawLine(lastPoint,endPoint);
        break;
    }
    case Arrow:
    {
        float x1 = lastPoint.x();  //lastPoint 起点

        float y1 = lastPoint.y();

        float x2 = endPoint.x(); //endPoint 终点

        float y2 = endPoint.y();

        float l = 10.0; //箭头的那长度

        float a = 0.5; //箭头与线段角度

        float x3 = x2 - l * cos(atan2((y2 - y1) , (x2 - x1)) - a);

        float y3 = y2 - l * sin(atan2((y2 - y1) , (x2 - x1)) - a);

        float x4 = x2 - l * sin(atan2((x2 - x1) , (y2 - y1)) - a);

        float y4 = y2 - l * cos(atan2((x2 - x1) , (y2 - y1)) - a);

        pp.drawLine(x2,y2,x3,y3);

        pp.drawLine(x2,y2,x4,y4);

        pp.drawLine(lastPoint/scale,endPoint/scale);
        break;
    }
    case Rectangle:
    {
        pp.drawRect(x,y,w,h);
        break;
    }
    case Ellipse:
    {
        pp.drawEllipse(x,y,w,h);
        break;
    }
    case TextInput:
    {
        pp.setFont(QFont("微软雅黑",this->m_Fontsize));//设置字体
        //QDEBUG()<< "@@@ TextInput:"<<this->m_inputText;
        //QDEBUG()<< "@@@ TextInput:"<<this->m_rc;
        pp.drawText(this->m_rc,Qt::TextWrapAnywhere,this->m_inputText);
        break;
    }
    default:
        //未找到 直接返回
        return;
    }


    setPixmap(*fullpixmap);
    //update(); //进行更新界面显示，可引起窗口重绘事件，重绘窗口
    drawEditUpdatePixmap(*fullpixmap);

    if(isrecord)
    {
        //加入操作队列中
        QPixmap* opitem = new QPixmap();
        *opitem = fullpixmap->copy();
        this->m_operStack.append(opitem);
    }

    //modified = true;
}

