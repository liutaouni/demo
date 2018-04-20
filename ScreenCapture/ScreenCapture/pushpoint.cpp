#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

#include "pushpoint.h"
#include "global.h"

PushPoint::PushPoint(QWidget *parent):
    location(TopLeft)
{
    setParent(parent);
    setWindowFlags(Qt::FramelessWindowHint);  //使窗口标题栏隐藏
    setMouseTracking(true);
    QPixmap pixmap = QPixmap(QSize(POINT_SIZE,POINT_SIZE));  //构建一个大小为600x400的QPixmap
    pixmap.fill(QColor(255,255,0));   //用红色填充这个pixmap Qt::red
    setPixmap(pixmap);
    show();
    this->m_pointCanMove = true;
}

PushPoint::~PushPoint()
{

}

void PushPoint::setLocation(LocationPoint Loa)
{
    location = Loa;
    setMouseCursor();
}

void PushPoint::setPointMoveState(bool state)
{
    this->m_pointCanMove = state;
    if(state==false)
    {
        setCursor(Qt::ArrowCursor);
    }
    else
    {
        setMouseCursor();
    }

}

void PushPoint::mousePressEvent(QMouseEvent * event)
{
    if(this->m_pointCanMove == false)
    {
        return;
    }
    if (event->button() == Qt::LeftButton)
    {
         dragPosition=event->globalPos()-frameGeometry().topLeft();
         event->accept();
    }
}
void PushPoint::mouseMoveEvent(QMouseEvent * event)
{
    if(this->m_pointCanMove == false)
    {
        return;
    }
    if (event->buttons() & Qt::LeftButton)
    {
        QPoint point;
        int x;
        int y;
        switch (location)
        {
        case TopLeft:
        case BottomRight:
        case TopRight:
        case BottomLeft:
            point = event->globalPos() - dragPosition;
            break;
        case TopMid:
        case BottomMid:
            y = (event->globalPos() - dragPosition).y();
            x = pos().x();
            point = QPoint(x,y);
            break;
        case MidLeft:
        case MidRight:
            x = (event->globalPos() - dragPosition).x();
            y = pos().y();
            point = QPoint(x,y);
            break;
        default:
            point = event->globalPos() - dragPosition;
            break;
        }
        //move(point);
        //QDEBUG()<<"emit moved"<< QDateTime::currentDateTime().toString("hh:mm:ss:zzz");
        emit moved(point);
        event->accept();
    }
}

void PushPoint::setMouseCursor()
{
    switch (location)
    {
    case TopLeft:
    case BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case TopRight:
    case BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case TopMid:
    case BottomMid:
        setCursor(Qt::SizeVerCursor);
        break;
    case MidLeft:
    case MidRight:
        setCursor(Qt::SizeHorCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}
