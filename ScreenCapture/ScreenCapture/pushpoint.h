#ifndef PUSHPOINT_H
#define PUSHPOINT_H

#include <QLabel>
#include <QMouseEvent>

/***
 ***改变范围时，显示的边框中的八个红点
 ***其中的一个点的类
 ***/
class PushPoint : public QLabel
{
    Q_OBJECT

public:
    explicit PushPoint(QWidget *parent = 0);
    ~PushPoint();
    enum LocationPoint //此点所在的位置
    {
        TopLeft = 0, //左上
        TopRight= 1, //右上
        TopMid  = 2, //上中
        BottomLeft= 3, //左下
        BottomRight= 4, //右下
        BottomMid= 5, //下中
        MidLeft= 6, //左中
        MidRight= 7 //右中
    };
    void setLocation(LocationPoint);
    LocationPoint locPoint(){return location;}
    void setPointMoveState(bool state);
protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

signals:
    void moved(QPoint);
private:
    QPoint dragPosition;
    bool m_pointCanMove;
    LocationPoint location; //此点的位置

    void setMouseCursor();
};

#endif // PUSHPOINT_H
