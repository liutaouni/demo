#include <QApplication>

#ifdef Q_OS_WIN
#include "ScreenshotWidget.h"
#else
#include "ScreenshotWidgetmac.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ScreenshotWidget w;
    w.connect(&w, &ScreenshotWidget::sigShotScreenFinished, &a, &QApplication::quit);

#ifdef Q_OS_WIN
    w.getReadyToShot();
    w.show();
#endif

    return a.exec();
}
