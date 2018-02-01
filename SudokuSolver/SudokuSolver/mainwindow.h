#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class Cell;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_startBtn_clicked();

    void on_clearBtn_clicked();

private:
    Ui::MainWindow *ui;

    Cell *m_cellArr[9][9];
};

#endif // MAINWINDOW_H
