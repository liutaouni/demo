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
    void paintEvent(QPaintEvent *e);

private:
    Ui::ComWindow *ui;
};

#endif // COMWINDOW_H
