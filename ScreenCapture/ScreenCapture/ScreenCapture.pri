#-------------------------------------------------
#
# Project created by QtCreator 2014-03-14T18:02:26
#
#-------------------------------------------------
INCLUDEPATH += $$PWD

macx {
    HEADERS += $$PWD/screenshotwidgetmac.h
    SOURCES += $$PWD/screenshotwidgetmac.cpp
}

win32 {
    SOURCES +=\
        $$PWD/pushpoint.cpp \
        $$PWD/selectrect.cpp \
        $$PWD/screenshotwidget.cpp

    HEADERS  += \
        $$PWD/pushpoint.h \
        $$PWD/global.h \
        $$PWD/selectrect.h \
        $$PWD/screenshotwidget.h
}

FORMS    += \    
    $$PWD/toolwidget.ui

RESOURCES += \
    $$PWD/snapshot_res.qrc


