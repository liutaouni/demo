#ifndef SCREENSHOTWIDGET_H
#define SCREENSHOTWIDGET_H

#include <QWidget>
#include <QGuiApplication>
#include <QLabel>
#include <QPixmap>
#include <QScreen>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QDesktopWidget>
#include <QPoint>
#include <QPaintEvent>
#include <QPushButton>
#include <QTextEdit>
#include <QList>
#include <QKeyEvent>

namespace Ui {
class ToolWidget;
}

enum directionStyle{
    m_SizeLeft=0, //水平左右
    m_SizeRight,
    m_SizeTop, //竖直上下
    m_SizeBottom,
    m_SizeLeftTop, //左上 右上
    m_SizeRightTop,
    m_SizeLeftBottom,//左下 右下
    m_SizeRightBottom
};

enum paintContent
{
    p_Rect=0,  //矩形
    p_Ellipse, //椭圆
    p_Arrow,   //箭头
    p_Brush,   //画刷
    p_Pen      //字体
};

class directionBtn : public QPushButton{
    Q_OBJECT
public:
    explicit directionBtn(QWidget *parent = 0, directionStyle directS=m_SizeLeft);
    ~directionBtn();
    void setBtnCursor();
protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
signals:
    void sig_directChangePos(directionStyle, QPoint);
private:
    bool m_pressed;
    QPoint m_changedPos, m_endPos,m_startPos;
    directionStyle m_directS;

};

class selectRect : public QLabel{
    Q_OBJECT
public:
    explicit selectRect(QWidget *parent);
    ~selectRect();
     void reFixWidget(QRect rect);
signals:
     void sigMovingRect(QRect rect);
protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
public slots:
    void slot_directChangePos(directionStyle, QPoint pt);
    void slotModified(bool);
private slots:

private:
    directionBtn *topLeftBtn;
    directionBtn *topMidBtn;
    directionBtn *topRightBtn;
    directionBtn *midLeftBtn;
    directionBtn *midRighttBtn;
    directionBtn *bottomLeftBtn;
    directionBtn *bottomMidBtn;
    directionBtn *bottomRightBtn;

    bool isMoving, isModified;
    QPoint startPoint, endPoint;
    QRect m_rect;

    void setDirectionBtnEnabled(bool mask=true);
};

class ScreenshotWidget : public QLabel
{
    Q_OBJECT
public:
    explicit ScreenshotWidget(QWidget *parent = 0);
    ~ScreenshotWidget();

protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void keyPressEvent(QKeyEvent *ev);

    void showEvent(QShowEvent *ev);
    void paintEvent(QPaintEvent *ev);
    void paint(QPixmap &pixmap);
    void closeEvent(QCloseEvent *ev);
signals:
    void sigShotScreenFinished(bool);
    void sigModified(bool);
private slots:
    void slotMovingRect(QRect rect);
    void slotSaveBtnClick();
    void slotCloseBtnClick();
    void slotOkbtnClick();
    void slotRectangleBtnClick();
    void slotEllipseBtnClick();
    void slotPaintBtnClick();
    void slotArrowBtnClick();
    void slotTexteditBtnClick();

    void slotSmallWidthBtnClick();
    void slotMidWidthBtnClick();
    void slotBigWidthBtnClick();
    void slotColorlableClick();
    void slotInputEditTextChanged();
    void slotCancelBtnClick();
private:
    void initToolWidget();
    void showToolWidget(bool mask);
    void WriteToClipboard(QPixmap pixmap);
    void doWithTextEdit();
    void updateTextEditState();
    void resetScreenShot();
    void showPositionStateWidget();
    void readyToPaint(paintContent content);
    int maxLineCountOfBlock();

     bool isDrawing, isModified, isFirstPen;
     QPixmap fullScreenPix, fullScreenTempPix, tempPix;
     QPoint startPoint,endPoint;
     QPoint lastStartPoint, lastEndPoint;
     QRect shotRect;
     selectRect *selectWidget;
     Ui::ToolWidget *ui;
     QWidget *m_toolWidget; //选中完毕后弹出的操作按钮
     QPixmap *m_pixmap; //截取得到的图像(高亮显示的图像)
     int i, m_paintBrush;
     paintContent m_content;

     QTextEdit* m_edit;
     int m_PenWidth; //笔宽
     int m_Fontsize; //字号
     QColor m_PenColor; //前景色

     QLabel *positionStateLabel; //选择区域位置大小信息
     QList<QPixmap> operateList; //编辑队列

};

#endif // SCREENSHOTWIDGET_H
