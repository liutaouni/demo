#ifndef SUDOKU_H
#define SUDOKU_H

#include <QList>

struct SudokuItem
{
    int m_num = 0;
    QList<int> m_possibleNums;

    SudokuItem()
    {
        m_possibleNums << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
    }
};

class Sudoku
{
public:
    static bool solve(int arr[][9]);

private:
    Sudoku();

    static void getSudokuIndexRange(int x, int y, int &xRangeMin, int &xRangeMax, int &yRangeMin, int &yRangeMax);
    static bool enterNum(int x, int y, int num, QList<SudokuItem> &sudoku);
    static bool calNextNum(QList<SudokuItem> &sudoku);
};

#endif // SUDOKU_H
