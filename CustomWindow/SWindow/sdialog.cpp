#include "sdialog.h"

#include <QVBoxLayout>
#include <QEvent>

#include <QDebug>

#include <windows.h>

SDialog::SDialog(QWidget *parent) : QDialog(parent)
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

QWidget *SDialog::getContentWidget()
{
    return mComWin->getContentWidget();
}

bool SDialog::nativeEvent(const QByteArray &eventType, void *message, long *result)
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

void SDialog::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::WindowStateChange)
    {
        mComWin->updateWindowStyle(this->isActiveWindow());
    }

    QWidget::changeEvent(event);
}
