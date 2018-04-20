#include "ScreenshotWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ScreenshotWidget w;
    w.connect(&w, &ScreenshotWidget::sigShotScreenFinished, &a, &QApplication::quit);

    w.getReadyToShot();
    w.show();

    return a.exec();
}
