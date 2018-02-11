#ifndef COMWINDOW_H
#define COMWINDOW_H

#include <QWidget>

namespace Ui {
class ComWindow;
}

class ComWindow : public QWidget
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
    explicit ComWindow(QWidget *parent = 0);
    ~ComWindow();

    QWidget *getContentWidget();

    void showActive();
    void showInactive();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    Ui::ComWindow *ui;

    bool mIsLeftBtnPressed = false;
    QPoint mLeftBtnPressedPos;
    QRect mWinGeometry;

    EBorderDragRegion mDragRegion = EBorderNone;

    void updateCorsurStyleForDragBorder(const QPoint &pos);
    void updateGeometryByDragBorder(const QPoint &pos);
};

#endif // COMWINDOW_H
