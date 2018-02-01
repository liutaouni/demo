#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "cell.h"
#include "sudoku.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            Cell *cell = new Cell(i, j);
            ui->gridLayout->addWidget(cell, i, j);
            m_cellArr[i][j] = cell;
        }
    }

//    m_cellArr[0][3]->setNum(5);
//    m_cellArr[0][4]->setNum(4);
//    m_cellArr[0][5]->setNum(2);

//    m_cellArr[1][1]->setNum(2);
//    m_cellArr[1][5]->setNum(7);
//    m_cellArr[1][6]->setNum(8);
//    m_cellArr[1][7]->setNum(4);

//    m_cellArr[2][1]->setNum(7);
//    m_cellArr[2][5]->setNum(1);

//    m_cellArr[3][0]->setNum(2);
//    m_cellArr[3][1]->setNum(6);
//    m_cellArr[3][2]->setNum(9);
//    m_cellArr[3][4]->setNum(1);
//    m_cellArr[3][8]->setNum(7);

//    m_cellArr[4][0]->setNum(8);
//    m_cellArr[4][3]->setNum(3);
//    m_cellArr[4][4]->setNum(2);
//    m_cellArr[4][5]->setNum(6);
//    m_cellArr[4][8]->setNum(5);

//    m_cellArr[5][0]->setNum(5);
//    m_cellArr[5][4]->setNum(9);
//    m_cellArr[5][6]->setNum(2);
//    m_cellArr[5][7]->setNum(1);
//    m_cellArr[5][8]->setNum(6);

//    m_cellArr[6][3]->setNum(8);
//    m_cellArr[6][7]->setNum(7);

//    m_cellArr[7][1]->setNum(8);
//    m_cellArr[7][2]->setNum(3);
//    m_cellArr[7][3]->setNum(2);
//    m_cellArr[7][7]->setNum(5);

//    m_cellArr[8][3]->setNum(1);
//    m_cellArr[8][4]->setNum(6);
//    m_cellArr[8][5]->setNum(4);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startBtn_clicked()
{
    int sudoku[9][9] = {0};

    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            sudoku[i][j] = m_cellArr[i][j]->getNum();
        }
    }

    Sudoku::solve(sudoku);

    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            m_cellArr[i][j]->setNum(sudoku[i][j]);
        }
    }
}

void MainWindow::on_clearBtn_clicked()
{
    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            m_cellArr[i][j]->clearNum();
        }
    }
}
