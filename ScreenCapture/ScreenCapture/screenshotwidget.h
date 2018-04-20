#ifndef SCREENSHOTWIDGET_H
#define SCREENSHOTWIDGET_H

#include "selectrect.h"

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QTextEdit>

#ifdef Q_OS_WIN32
#include <winsock2.h>
#include <windows.h>
#define SHOT_SCREEN_AUTO_IDENTIFY_WIN 1
#endif

/***  截图时显示的控件,主要包含三部分:
 ***1.显示全屏截图图像的label
 ***2.显示阴影部分的label
 ***3.显示高亮图像的label,(包含了:label、8个点、以及操作按钮)
 ***
 ***/

namespace Ui {
class ToolWidget;
}

class ScreenshotWidget : public QLabel
{
    Q_OBJECT

public:
    explicit ScreenshotWidget(QWidget *parent = 0);
    ~ScreenshotWidget();
    enum ShotState //截图状态
    {
        READY    = 0, //准备开始截图
        SHOTTING = 1, //正在截图
        FINISHED = 2  //截图完毕
    };
    enum ShapeType
    {     None,
          Line,
          Rectangle,
          Ellipse,
          Arrow,
          Painter,
          TextInput,
    };
    void getReadyToShot();//设置好所有的label准备截图
    ShotState getState(){return m_State;}

protected:
    void mousePressEvent(QMouseEvent * event);
    void keyReleaseEvent(QKeyEvent *e);
signals:
    void sigShotScreenFinished(bool isok);
private:
    Ui::ToolWidget *ui;

    QPoint m_oldCursorPos;
    //QPoint dragPosition;
    ShotState m_State; //状态

    /*自动选择的区域*/
    QRect m_autoRect;
    QTimer *m_autoRectTimer;       //定时更新当前鼠标所在点的窗口的信息

    /*显示图像*/
    QPixmap *m_fullScreenPixmap; //截取全屏的图像
    QPixmap *m_pixmap; //截取得到的图像(高亮显示的图像)
    QLabel *m_pixLabel; //显示高亮图像的label
    QLabel *m_label;  //显示出阴影效果的label
    QWidget *m_toolWidget; //选中完毕后弹出的操作按钮

    QLabel *m_infolabel;  //显示截图信息的label
    QPixmap *m_infoPix;       // 截图信息显示背景

    /*选择范围*/
    SelectRect *m_selectRect;
    QRect m_rect;

    ShapeType m_curShape;
    int m_PenWidth;
    QColor m_PenColor;
    int m_Fontsize;

    QTextEdit* m_edit;
    QString m_inputText;
    QRect m_rc;

    QList<QPixmap*> m_operStack;//撤销操作 队列

    void initToolWidget();
    void showToolWidget();

    void updateAutoPix(); //更新显示自动选择的区域
    #ifdef SHOT_SCREEN_AUTO_IDENTIFY_WIN
    QRect getCursorWindowInfo();//获取鼠标所在位置的窗口的信息(使用windowsAPI)
    #endif
    void updatePixmap();  //更新高亮显示的图像
    void finishShot();    //截图完毕


    void drawEditUpdatePixmap(QPixmap editpixmap);
    void showPenSettingPanel(ShapeType oldtype,ShapeType newtype);
    void inputTextEditFocusout();
    void setTextInputEditFontSize();

    void ScreenShotSaveClipboard(QPixmap);
    void WriteToClipboard(QPixmap);

    int maxLineCountOfBlock();

private slots:
    void slotAutoPixSelect();
    void slotRectChanged(QRect);
    void slotRectSelectFinished(QRect);

    void slotSaveBtnClick(); //保存截图
    void slotCancelBtnClick();
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

    void slotInputEditTextChange();
    void slotColorlableClick();
    #ifdef SHOT_SCREEN_AUTO_IDENTIFY_WIN
    void slotUpdateCusorWindowInfo(); //定时更新当前鼠标所在点的窗口的信息
    #endif
    void paint(bool isend);
    void paint(QPixmap* fullpixmap,bool isrecord);
};
#endif // SCREENSHOTWIDGET_H
