#include "widget.h"
#include <QApplication>

#include "translator.h"
#include "zhtoen.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Translator tr;

    Widget w_zh;
    w_zh.show();

    QObject::connect(&tr, &Translator::sigTranslateResult, &w_zh, &Widget::slotShowTranslateResult);

    zhToEn toEn;

    Widget w_en;
    w_en.show();

    QObject::connect(&tr, &Translator::sigTranslateResult, &toEn, &zhToEn::slotTranslateResult);
    QObject::connect(&toEn, &zhToEn::sigFinished, &w_en, &Widget::slotShowTranslateResult);

    return a.exec();
}
