#include "cell.h"

#include <QStyleOption>
#include <QPainter>
#include <QRegExpValidator>

Cell::Cell(int x, int y, QWidget *parent) : QLineEdit(parent)
{
    this->setFixedSize(50, 50);
    this->setValidator(new QIntValidator(1, 9, this));
    this->setAlignment(Qt::AlignCenter);

    int borderLeft = 1;
    int borderTop = 1;
    int borderRight = 0;
    int borderBottom = 0;

    if(x == 0){
        borderTop = 2;
    }

    if(y == 0){
        borderLeft = 2;
    }

    if(y == 8){
        borderRight = 2;
    }

    if(x == 8){
        borderBottom = 2;
    }

    if(x == 3 || x == 6){
        borderTop = 2;
    }

    if(y == 3 || y == 6){
        borderLeft = 2;
    }

    QString bgColor = "white";

    if((x == 0 || x == 1 || x == 2) && (y == 3 || y == 4 || y == 5)){
        bgColor = "lightGray";
    }

    if((x == 3 || x == 4 || x == 5) && (y == 0 || y == 1 || y == 2 || y == 6 || y == 7 || y == 8))
    {
        bgColor = "lightGray";
    }

    if((x == 6 || x == 7 || x == 8) && (y == 3 || y == 4 || y == 5)){
        bgColor = "lightGray";
    }

    QString style = QString("QLineEdit{"
                            "border-left:%1px solid black; "
                            "border-top:%2px solid black; "
                            "border-right:%3px solid black; "
                            "border-bottom:%4px solid black;"
                            "background:%5;"
                            "font:bold 36px;"
                            "color:black;}")
            .arg(borderLeft).arg(borderTop).arg(borderRight).arg(borderBottom).arg(bgColor);
    this->setStyleSheet(style);
}

void Cell::paintEvent(QPaintEvent *e)
{
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive( QStyle::PE_Widget, &o, &p, this);
    QLineEdit::paintEvent(e);
}

int Cell::getNum()
{
    return this->text().toInt();
}

void Cell::setNum(int num)
{
    this->setText(QString::number(num));
}

void  Cell::clearNum()
{
    this->setText("");
}
