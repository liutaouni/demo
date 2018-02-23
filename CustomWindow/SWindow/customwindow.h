#ifndef CUSTOMWINDOW_H
#define CUSTOMWINDOW_H

#include <QWidget>

namespace Ui {
class CustomWindow;
}

class CustomWindow : public QWidget
{
    Q_OBJECT

private:
    enum EBorderDragRegion
    {
        EBorderLeft,
        EBorderTop,
        EBorderRight,
        EBorderBottom,
        EBorderTopLeft,
        EBorderTopRight,
        EBorderBottomLeft,
        EBorderBottomRight,
        EBorderNone
    };

public:
    explicit CustomWindow(QWidget *parent = 0);
    ~CustomWindow();

    QWidget *getContentWidget();

    void updateWindowStyle(bool isActive);
    void updateWindowTitle(const QString &title);
    void updateWindowIcon(const QIcon &icon);

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);

private slots:

private:
    Ui::CustomWindow *ui;

    const int mBorderWidth = 7;

    bool mIsLeftBtnPressed = false;
    QPoint mLeftBtnPressedPos;
    QPoint mLeftBtnPressedGlobalPos;
    QRect mWinGeometryPressed;
    QSize mWinSizePre;
    QSize mWinSizeCur;
    EBorderDragRegion mDragRegion = EBorderNone;

    void updateCorsurStyleForDragBorder(const QPoint &pos);
    void updateGeometryByDragBorder(const QPoint &pos);
};

#endif // CUSTOMWINDOW_H
