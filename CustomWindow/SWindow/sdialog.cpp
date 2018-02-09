#include "sdialog.h"

SDialog::SDialog(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
}
