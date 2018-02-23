#include "swindow.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QEvent>
#include <QIcon>

#include <windows.h>

SWindow::SWindow(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

#ifdef USE_CUSTOM_WINDOW
    mComWin = new CustomWindow(this);
#elif defind(Q_OS_WIN)
    mComWin = new ComWindow(this);
#elif defind(Q_OS_MAC)

#else

#endif

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mComWin);

    this->setLayout(layout);

    this->setWindowTitle(tr("Form"));
    this->setWindowIcon(QIcon(":/icon/icon/window_icon.png"));
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
                mComWin->updateWindowStyle(false);
            }
            else
            {
                mComWin->updateWindowStyle(true);
            }
        }
    }

    return QWidget::nativeEvent(eventType,message,result);
}

bool SWindow::event(QEvent *event)
{
    if(event->type() == QEvent::WindowIconChange)
    {
        mComWin->updateWindowIcon(this->windowIcon());
    }

    return QWidget::event(event);
}

void SWindow::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::WindowStateChange)
    {
        mComWin->updateWindowStyle(this->isActiveWindow());
    }
    else if(event->type() == QEvent::WindowTitleChange)
    {
        mComWin->updateWindowTitle(this->windowTitle());
    }

    QWidget::changeEvent(event);
}
