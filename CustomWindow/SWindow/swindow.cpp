#include "swindow.h"

#include <QVBoxLayout>

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
