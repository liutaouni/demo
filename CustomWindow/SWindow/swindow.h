#ifndef SWINDOW_H
#define SWINDOW_H

#include <QWidget>

#include "comwindow_win.h"
#include "customwindow.h"

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
#ifdef USE_CUSTOM_WINDOW
    CustomWindow *mComWin = NULL;
#elif defined Q_OS_WIN
    ComWindow_Win *mComWin = NULL;
#elif defined Q_OS_MAC

#else

#endif
};

#endif // SWINDOW_H
