#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

#include "selectrect.h"
#include <QPainter>
#include "global.h"
#include <QBitmap>
SelectRect::SelectRect(QWidget *parent,Mode m) :
    QLabel(parent),m_modified(false),m_isDrawing(false),
    runningMode(m)
{
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);  //使窗口额标题栏隐藏并置顶
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("background:transparent;");
    setMouseTracking(true);
    hide();
    initLocationPoint();
}

SelectRect::~SelectRect()
{
    for (int i = 0; i < 8; i++)
    {
        delete locPoint[i];
    }
}

void SelectRect::getReadyToSelect()
{
    showFullScreen();
    setCursor(Qt::ArrowCursor);
    QPixmap pix = QPixmap(this->size());
    pix.fill(QColor(0,0,0,1));
    setPixmap(pix);

    FINISHED = false;
    this->m_isDrawing = false;
    setDrawEditModel(false);

    int i;
    for (i = 0; i < 8; i++)
    {
        locPoint[i]->move(-100,-100);
    }
}

void SelectRect::setDrawEditModel(bool ismodified)
{
    this->m_modified = ismodified;
    setLocationPointMoveState(!ismodified);
}

QPoint SelectRect::getLastPoint()
{
    return this->m_lastPoint;
}

void SelectRect::setLastPoint(QPoint point)
{
    this->m_lastPoint = point;
}

QPoint SelectRect::getEndPoint()
{
    return this->m_endPoint;
}

void SelectRect::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (runningMode == ScreenShot)
    {
        if (event->button() == Qt::LeftButton)
        {
            if (rect.contains(event->pos()))
            {
                event->ignore();
            }
        }
    }
    else if(runningMode == RecordGif)
    {
        if (rect.contains(event->pos()))
        {
            QPixmap pix = QPixmap(this->size());;
            pix.fill(QColor(0,0,0,0));

            QPainter painter(&pix);
            painter.setPen(QPen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap));
            painter.drawRect(rectangle);
            setPixmap(pix);

            int i;
            for (i = 0; i < 8; i++)
            {
                locPoint[i]->move(-100,-100);
            }
            emit sigSelectrectFinished(rect);
        }
        event->accept();
    }

}

void SelectRect::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
    {
        //QDEBUG() << "mousePressEvent";
        this->m_lastPoint = event->pos();
        if(this->m_modified)
        {
            if (rect.contains(event->globalPos()))
            {
                this->m_isDrawing = true;
            }
            return;
        }

        RELEASE = true;
        if (FINISHED == false)
        {
            rect.setLeft(event->x());
            rect.setTop(event->y());
            event->accept();
        }
        else
        {
            if (rect.contains(event->globalPos()))
            {
                dragPosition = event->globalPos()-frameGeometry().topLeft();
                zeroPoint = rect.topLeft();
                DRAG = true;
            }
            else
            {
                DRAG = false;
            }
        }

    }
    else if (event->button() == Qt::RightButton)
    {
        event->ignore();
    }
}
void SelectRect::mouseMoveEvent(QMouseEvent * event)
{
    RELEASE = false;
    if (event->buttons() & Qt::LeftButton)
    {
        if(this->m_modified)
        {
            //setCursor(Qt::CrossCursor);
            setCustomCursor();
            if(this->m_isDrawing)
            {
                if (rect.contains(event->pos()))
                {
                    this->m_endPoint = event->pos();
                    //   paint(image);
                    emit sigPaint(false);
                }
                else
                {
                    //QDEBUG()<<"mouseMoveEvent out";
                    //this->m_endPoint = event->pos();
                    this->m_isDrawing = false;
                    //paint();
                    emit sigPaint(true);
                }
            }

            return;
        }

        if (FINISHED == false)
        {
            int mx,mw,my,mh;

            if(event->x() < this->m_lastPoint.x())
            {
                mx = event->x();
                mw = this->m_lastPoint.x() - event->x();
            }
            else
            {
                mx = this->m_lastPoint.x();
                mw = event->x() - this->m_lastPoint.x();
            }
            rect.setLeft(mx);
            rect.setWidth(mw);
            if(event->y() < this->m_lastPoint.y())
            {
                my = event->y();
                mh = this->m_lastPoint.y() - event->y();
            }
            else
            {
                my = this->m_lastPoint.y();
                mh = event->y() - this->m_lastPoint.y();
            }
            rect.setTop(my);
            rect.setHeight(mh);
            //QDEBUG()<<"rect="<<rect;
            updateRect();
        }
        else
        {
            if ( DRAG == true)
            {
                int width = rect.width();
                int height= rect.height();

                QPoint p =event->globalPos() - dragPosition + zeroPoint;
                if(p.x() < 0) p.setX(0);
                if(p.y() < 0) p.setY(0);
                if((p.x()+width)>this->width()) p.setX(this->width()-width);
                if((p.y()+height)>this->height())p.setY(this->height()-height);

                rect.setTopLeft(p);
                rect.setWidth(width);
                rect.setHeight(height);
                updateRect();
            }
        }
        emit sigRectChanged(rect);
    }

    if (FINISHED == true)
    {
        if (rect.contains(event->pos()))
        {
            if(this->m_modified)
            {
                setCustomCursor();
                //setCursor(Qt::CrossCursor);
            }
            else
            {
                setCursor(Qt::SizeAllCursor);
            }
        }
        else
        {
            setCursor(Qt::ArrowCursor);
        }
    }

}

void SelectRect::mouseReleaseEvent(QMouseEvent * event)
{
    if (this->isVisible() && event->button() == Qt::LeftButton)
    {
        //QDEBUG()<<"@@@@@mouseReleaseEvent FINISHED=" <<FINISHED << RELEASE;
        if(this->m_modified)
        {
            if(this->m_isDrawing)
            {
                //QDEBUG()<<"mouseReleaseEvent";
                this->m_endPoint = event->pos();
                this->m_isDrawing = false;
                //paint();
                emit sigPaint(true);
            }
            return;
        }
        if (FINISHED)
        {
            return;
        }
        if (RELEASE)
        {
            emit sigReleaseWithoutMove();
            FINISHED = true;
            return;
        }
        int minWidth = POINT_SIZE;
        if (rect.width() < minWidth && rect.height() < minWidth)
        { //防止手抖动,不小心选到了一个很小的区域
            emit sigReleaseWithoutMove();
        }
        else
        {
            FINISHED = true;
            //QDEBUG()<<"@@@@@ FINISHED=true";
            emit sigSelectrectFinished(rect);
        }
    }
}


void SelectRect::initLocationPoint()
{
    int i;
    for (i = 0; i < 8; i++)
    {
        locPoint[i] = new PushPoint(this);
        locPoint[i]->setLocation((PushPoint::LocationPoint)i);
        connect(locPoint[i],SIGNAL(moved(QPoint)),this,SLOT(slotPointMoved(QPoint)));
        locPoint[i]->move(-100,-100);
    }
}

void SelectRect::setLocationPointMoveState(bool state)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        locPoint[i]->setPointMoveState(state);
    }
}


void SelectRect::layoutLocPoint()
{

    int start_x = rectangle.x();
    int start_y = rectangle.y();
    int P_width = locPoint[0]->width();
    int p_height= locPoint[0]->height();
    int x = rectangle.x() + rectangle.width();
    int y = rectangle.y() + rectangle.height();
    int half_x = rectangle.x() + (rectangle.width()-P_width) / 2;//一半
    int half_y = rectangle.y() + (rectangle.height()-p_height) / 2;//一半

    locPoint[PushPoint::TopLeft]->move(start_x,start_y);
    locPoint[PushPoint::TopRight]->move(x - P_width,start_y);
    locPoint[PushPoint::TopMid]->move(half_x,start_y);;
    locPoint[PushPoint::BottomLeft]->move(start_x,y - p_height);
    locPoint[PushPoint::BottomRight]->move(x - P_width,y - p_height);
    locPoint[PushPoint::BottomMid]->move(half_x,y - p_height);
    locPoint[PushPoint::MidLeft]->move(start_x,half_y);
    locPoint[PushPoint::MidRight]->move(x - P_width,half_y);
}

void SelectRect::slotPointMoved(QPoint p)
{
    PushPoint * curPoint = qobject_cast<PushPoint *>(sender());
    if(!curPoint)
        return;
    int lpoint = curPoint->locPoint();
    int width = rect.width();
    int height = rect.height();
    int x = p.x();
    int y = p.y();
    if (x < 0) x=0;
    if (y < 0) y=0;
    int minWidth = curPoint->width();
    int minHeight= curPoint->height();
    if (lpoint == PushPoint::TopLeft)
    {
        int BR_x = rect.x() + width; //右下方点的坐标
        int BR_y = rect.y() + height; //右下方点的坐标
        if (x > (BR_x - minWidth))
            x = BR_x - minWidth;
        if (y > (BR_y - minHeight))
            y = BR_y - minHeight;

        curPoint->move(x,y);
        width = BR_x - x;
        height= BR_y - y;
        rect.setX(BR_x - width);
        rect.setY(BR_y - height);
    }
    else if (lpoint == PushPoint::TopRight)
    {
        int BL_x = rect.x(); //左下方点的坐标
        int BL_y = rect.y() + height; //左下方点的坐标
        if (x < (BL_x + minWidth))
            x = BL_x + minWidth;
        if (y > (BL_y - minHeight))
            y = BL_y - minHeight;
        curPoint->move(x,y);
        width = x - BL_x;
        height = BL_y - y;
        rect.setY(BL_y - height);
    }
    else if (lpoint == PushPoint::TopMid)
    {
        int B_y = rect.y() + height; //最下方的纵坐标
        if (y > (B_y - minHeight))
            y = B_y - minHeight;
        curPoint->move(x,y);
        height = B_y - y;
        rect.setY(B_y - height);
    }
    else if (lpoint == PushPoint::BottomLeft)
    {
        int TR_x = rect.x() + width; //右上方点的坐标
        int TR_y = rect.y(); //右上方点的坐标
        if (x > (TR_x - minWidth))
            x = TR_x - minWidth;
        if (y < (TR_y + minHeight))
            y = TR_y + minHeight;
        curPoint->move(x,y);
        width = TR_x - x;
        height= y - TR_y;
        rect.setX(TR_x - width);
    }
    else if (lpoint == PushPoint::BottomMid)
    {
        if (y < (rect.y() + minHeight))
            y = rect.y() + minHeight;
        curPoint->move(x,y);
        height = y - rect.y();
    }
    else if (lpoint == PushPoint::BottomRight)
    {
        if (x < (rect.x() + minWidth))
            x = rect.x() + minWidth;
        if (y < (rect.y() + minHeight))
            y = rect.y() + minHeight;
        curPoint->move(x,y);
        width = x - rect.x();
        height= y - rect.y();
    }
    else if (lpoint == PushPoint::MidLeft)
    {
        int R_x = rect.x() + width; //最右方点的横坐标
        if (x > (R_x - minWidth))
            x = R_x - minWidth;
        curPoint->move(x,y);
        width = R_x - x;
        rect.setX(R_x - width);
    }
    else if (lpoint == PushPoint::MidRight)
    {
        if (x < (rect.x() + minWidth))
            x = rect.x() + minWidth;
        curPoint->move(x,y);
        width = x - rect.x();
    }

    rect.setWidth(width);
    rect.setHeight(height);

    updateRect();
    //QDEBUG()<<"emit sigrectChanged"<< QDateTime::currentDateTime().toString("hh:mm:ss:zzz");
    emit sigRectChanged(rect);
}

void SelectRect::updateRect()
{
    rectangle.setTopLeft(QPoint(rect.x()-1,rect.y()-1));
    rectangle.setWidth(rect.width()+1);
    rectangle.setHeight(rect.height()+1);

    if (rectangle.x() < 0 || rectangle.y() < 0) rectangle = rect;

//    QPixmap pix = QPixmap(this->size());;
//    pix.fill(QColor(0,0,0,1));
//    QPainter painter(&pix);
//    painter.setPen(QPen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap));
//    painter.drawRect(rectangle);
//    setPixmap(pix);

    layoutLocPoint();
}

void SelectRect::setCustomCursor()
{
//    QBitmap pixmapCur(":/snapshotimages/cursor.jpg");
//    //QBitmap bitMap = pixmapCur.createMaskFromColor(QColor(Qt::red));
//    //pixmapCur.setMask(bitMap);   // 红色变透明

//    QCursor cur(pixmapCur,3,3);   // 热点是(3,3)

//    setCursor(cur);                        // 设置光标
    setCursor(Qt::ArrowCursor);//使用系统
}
