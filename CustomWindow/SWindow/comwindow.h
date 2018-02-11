#ifndef COMWINDOW_H
#define COMWINDOW_H

#include <QWidget>

namespace Ui {
class ComWindow;
}

class ComWindow : public QWidget
{
    Q_OBJECT

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

    bool mIsMouseLeftBtnPressed = false;

    void updateCorsurStyleForDragBorder(const QPoint &pos);
};

#endif // COMWINDOW_H
