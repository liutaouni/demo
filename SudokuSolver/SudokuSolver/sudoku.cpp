#include "sudoku.h"

#include <QDebug>

Sudoku::Sudoku()
{

}

bool Sudoku::solve(int arr[][9])
{
    QList<SudokuItem> sudosu;
    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            SudokuItem item;
            sudosu.append(item);
        }
    }

    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            if(arr[i][j] >= 1 && arr[i][j] <= 9)
            {
                if(enterNum(i, j, arr[i][j], sudosu) == false)
                {
                    return false;
                }
            }
        }
    }

    if(calNextNum(sudosu))
    {
        for(int i = 0; i < 9; i++)
        {
            for(int j = 0; j < 9; j++)
            {
                arr[i][j] = sudosu[i*9+j].m_num;
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}

void Sudoku::getSudokuIndexRange(int x, int y, int &xRangeMin, int &xRangeMax, int &yRangeMin, int &yRangeMax)
{
    switch(x)
    {
    case 0:
    case 1:
    case 2:
        xRangeMin = 0;
        xRangeMax = 2;
        break;
    case 3:
    case 4:
    case 5:
        xRangeMin = 3;
        xRangeMax = 5;
        break;
    case 6:
    case 7:
    case 8:
        xRangeMin = 6;
        xRangeMax = 8;
        break;
    }

    switch(y)
    {
    case 0:
    case 1:
    case 2:
        yRangeMin = 0;
        yRangeMax = 2;
        break;
    case 3:
    case 4:
    case 5:
        yRangeMin = 3;
        yRangeMax = 5;
        break;
    case 6:
    case 7:
    case 8:
        yRangeMin = 6;
        yRangeMax = 8;
        break;
    }
}

bool Sudoku::enterNum(int x, int y, int num, QList<SudokuItem> &itemList)
{
    QList<SudokuItem> tempSudoko = itemList;

    tempSudoko[x*9+y].m_num = num;

    for(int tempY = 0; tempY < 9; tempY++)
    {
        SudokuItem &item = tempSudoko[x*9+tempY];

        if(tempY != y && item.m_num == 0)
        {
            item.m_possibleNums.removeAll(num);

            if(item.m_possibleNums.count() == 0)
            {
                return false;
            }
        }
    }

    for(int tempX = 0; tempX < 9; tempX++)
    {
        SudokuItem &item = tempSudoko[tempX*9+y];

        if(tempX != x && item.m_num == 0)
        {
            item.m_possibleNums.removeAll(num);

            if(item.m_possibleNums.count() == 0)
            {
                return false;
            }
        }
    }

    int xRangeMin, xRangeMax, yRangeMin, yRangeMax;
    getSudokuIndexRange(x, y, xRangeMin, xRangeMax, yRangeMin, yRangeMax);

    for(int indexX = xRangeMin; indexX <= xRangeMax; indexX++)
    {
        for(int indexY = yRangeMin; indexY <= yRangeMax; indexY++)
        {
            SudokuItem &item = tempSudoko[indexX*9+indexY];

            if(indexX != x && indexY != y && item.m_num == 0)
            {
                item.m_possibleNums.removeAll(num);

                if(item.m_possibleNums.count() == 0)
                {
                    return false;
                }
            }
        }
    }

    itemList = tempSudoko;
    return true;
}

bool Sudoku::calNextNum(QList<SudokuItem> &soduku)
{
    int minPossibleCount = -1;
    int index_x = -1;
    int index_y = -1;

    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            const SudokuItem &curItem = soduku.at(i*9+j);
            if(curItem.m_num == 0)
            {
                if(minPossibleCount == -1)
                {
                    minPossibleCount = curItem.m_possibleNums.count();
                    index_x = i;
                    index_y = j;
                }
                else if(curItem.m_possibleNums.count() < minPossibleCount)
                {
                    minPossibleCount = curItem.m_possibleNums.count();
                    index_x = i;
                    index_y = j;
                }
            }
        }
    }

    if(index_x >= 0 && index_y >= 0 && minPossibleCount > 0)
    {
        const SudokuItem &curItem = soduku.at(index_x*9+index_y);

        for(int k = 0; k < curItem.m_possibleNums.count(); k++)
        {
            QList<SudokuItem> tempSudoku = soduku;

            if(enterNum(index_x, index_y, curItem.m_possibleNums.at(k), tempSudoku))
            {
                if(calNextNum(tempSudoku))
                {
                    soduku = tempSudoku;
                    return true;
                }
            }
        }

        return false;
    }
    else
    {
        qDebug() << "not find";
        return true;
    }
}
