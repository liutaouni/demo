#-------------------------------------------------
#
# Project created by QtCreator 2018-02-01T23:08:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SudokuSolver
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    cell.cpp \
    sudoku.cpp

HEADERS  += mainwindow.h \
    cell.h \
    sudoku.h

FORMS    += mainwindow.ui
