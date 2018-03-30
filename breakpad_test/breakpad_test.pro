#-------------------------------------------------
#
# Project created by QtCreator 2018-03-09T15:06:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = breakpad_test
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/breakpad/Windows/lib/ -lcommon -lcrash_generation_client -lexception_handler
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/breakpad/Windows/lib/ -lcommond -lcrash_generation_clientd -lexception_handlerd
win32:INCLUDEPATH += $$PWD/breakpad/Windows/include
win32:DEPENDPATH += $$PWD/breakpad/Windows/include

mac: LIBS += -F$$PWD/breakpad/Mac/lib/ -framework Breakpad
mac:INCLUDEPATH += $$PWD/breakpad/Mac/include
mac:DEPENDPATH += $$PWD/breakpad/Mac/include



