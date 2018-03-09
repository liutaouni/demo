#include "mainwindow.h"
#include <QApplication>

#include <QDebug>

#include "exception_handler.h"
#include "crash_report_sender.h"

// 程序崩溃回调函数;
bool callback(const wchar_t *dump_path, const wchar_t *id,
    void *context, EXCEPTION_POINTERS *exinfo,
    MDRawAssertionInfo *assertion,
    bool succeeded)
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
    google_breakpad::ExceptionHandler eh(
        L".", NULL, callback, NULL,
        google_breakpad::ExceptionHandler::HANDLER_ALL);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
