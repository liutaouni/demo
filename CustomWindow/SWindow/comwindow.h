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
    void on_minBtn_clicked();

    void on_maxBtn_clicked();

    void on_closeBtn_clicked();

private:
    Ui::ComWindow *ui;

    const int mBorderWidth = 7;
    const int mMaxTitleHeight = 23;
    const int mNorTitleHeight = 29;

    bool mIsLeftBtnPressed = false;
    QPoint mLeftBtnPressedPos;
    QRect mWinGeometry;

    EBorderDragRegion mDragRegion = EBorderNone;

    QString mWinTitle;

    void updateCorsurStyleForDragBorder(const QPoint &pos);
    void updateGeometryByDragBorder(const QPoint &pos);
};

#endif // COMWINDOW_H
