#ifndef CELL_H
#define CELL_H

#include <QLineEdit>

class Cell : public QLineEdit
{
    Q_OBJECT
public:
    explicit Cell(int x, int y, QWidget *parent = 0);

    int getNum();
    void setNum(int num);
    void clearNum();

protected:
    void paintEvent(QPaintEvent *e);
};

#endif // CELL_H
