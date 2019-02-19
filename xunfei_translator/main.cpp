#include "widget.h"
#include <QApplication>

#include "translator.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Translator tr;

    Widget w;
    w.show();

    QObject::connect(&tr, &Translator::sigTranslateResult, &w, &Widget::slotShowTranslateResult);

    return a.exec();
}
