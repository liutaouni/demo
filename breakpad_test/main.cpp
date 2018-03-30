#include "mainwindow.h"
#include <QApplication>

#include <QDebug>

#include "exception_handler.h"

// 程序崩溃回调函数;
#ifdef Q_OS_WIN
bool callback(const wchar_t *dump_path, const wchar_t *id,
    void *context, EXCEPTION_POINTERS *exinfo,
    MDRawAssertionInfo *assertion,
    bool succeeded)
#elif defined Q_OS_MAC
bool callback(const char *dump_path, const char *id,
    void *context, bool succeeded)
#else

#endif
{
    if (succeeded)
    {
        qDebug() << "Create dump file success";
    }
    else
    {
        qDebug() << "Create dump file failed";
    }
    return succeeded;
}

int main(int argc, char *argv[])
{
    // 创建捕捉程序异常对象;
#ifdef Q_OS_WIN
    google_breakpad::ExceptionHandler eh(
        L".", NULL, callback, NULL,
        google_breakpad::ExceptionHandler::HANDLER_ALL);
#elif defined Q_OS_MAC
    google_breakpad::ExceptionHandler eh(
        ".", NULL, callback, NULL,
        true, NULL);
#else

#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
