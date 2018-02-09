#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include "SWindow/sdialog.h"

namespace Ui {
class TestDialog;
}

class TestDialog : public SDialog
{
    Q_OBJECT

public:
    explicit TestDialog(QWidget *parent = 0);
    ~TestDialog();

private:
    Ui::TestDialog *ui;
};

#endif // TESTDIALOG_H
