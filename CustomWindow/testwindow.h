#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include "SWindow/swindow.h"

namespace Ui {
class TestWindow;
}

class TestWindow : public SWindow
{
    Q_OBJECT

public:
    explicit TestWindow(QWidget *parent = 0);
    ~TestWindow();

private:
    Ui::TestWindow *ui;
};

#endif // TESTWINDOW_H
