#-------------------------------------------------
#
# Project created by QtCreator 2018-02-09T16:17:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CustomWindow
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    testwindow.cpp \
    testdialog.cpp

HEADERS  += mainwindow.h \
    testwindow.h \
    testdialog.h

FORMS    += mainwindow.ui \
    testwindow.ui \
    testdialog.ui


include (SWindow/SWindow.pri)
