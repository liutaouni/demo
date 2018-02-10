#include "sdialog.h"

#include <QVBoxLayout>

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
