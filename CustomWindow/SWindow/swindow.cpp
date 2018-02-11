#include "swindow.h"

#include <QVBoxLayout>

#include <QDebug>

#include <windows.h>

SWindow::SWindow(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    mComWin = new ComWindow();

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mComWin);

    this->setLayout(layout);
}

QWidget *SWindow::getContentWidget()
{
    return mComWin->getContentWidget();
}

bool SWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    if (eventType == "windows_generic_MSG")
    {
        MSG* msg = static_cast<MSG *>(message);

        if (msg->message == WM_ACTIVATE)
        {
            if(msg->wParam == WA_INACTIVE)
            {
                mComWin->showInactive();
            }
            else
            {
                mComWin->showActive();
            }
        }
    }

    return QWidget::nativeEvent(eventType,message,result);
}
