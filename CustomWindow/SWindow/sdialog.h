#ifndef SDIALOG_H
#define SDIALOG_H

#include <QDialog>

#include "comwindow.h"

class SDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SDialog(QWidget *parent = 0);

    QWidget *getContentWidget();

signals:

public slots:

private:
    ComWindow *mComWin = NULL;
};

#endif // SDIALOG_H
