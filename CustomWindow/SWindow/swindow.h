#ifndef SWINDOW_H
#define SWINDOW_H

#include <QWidget>

#include "comwindow.h"

class SWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SWindow(QWidget *parent = 0);

    QWidget *getContentWidget();

signals:

public slots:

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    bool event(QEvent *event);
    void changeEvent(QEvent *event);

private:
    ComWindow *mComWin = NULL;
};

#endif // SWINDOW_H
