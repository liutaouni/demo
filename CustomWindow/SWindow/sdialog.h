#ifndef SDIALOG_H
#define SDIALOG_H

#include <QDialog>

#include "comwindow.h"
#include "customwindow.h"

class SDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SDialog(QWidget *parent = 0);

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
#elif defind(Q_OS_WIN)
    ComWindow *mComWin = NULL;
#elif defind(Q_OS_MAC)

#else

#endif
};

#endif // SDIALOG_H
