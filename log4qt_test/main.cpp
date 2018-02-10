#include "mainwindow.h"
#include <QApplication>

#include "log4qt/logmanager.h"
#include "log4qt/logger.h"
#include "log4qt/dailyrollingfileappender.h"
#include "log4qt/patternlayout.h"
#include <QTextCodec>

int main(int argc, char *argv[])
{
    // Create a layout
    Log4Qt::PatternLayout *layout = new Log4Qt::PatternLayout();
    layout->setConversionPattern("[%-23d{yyyy/MM/dd HH:mm:ss.zzz}] [%-5p] %m%n");
    layout->activateOptions();
    // Create an appender
    Log4Qt::DailyRollingFileAppender *appender = new Log4Qt::DailyRollingFileAppender();
    appender->setLayout(layout);
    appender->setFileNamePrefix("log_");
    appender->setDatePattern("yyyy-MM-dd-HH-mm'.txt'");
    appender->setThreshold(Log4Qt::Level::INFO_INT);
    appender->setImmediateFlush(true);
    appender->setAppendFile(true);
    appender->setEncoding(QTextCodec::codecForName("UTF-8"));
    appender->activateOptions();
    // Set appender on root logger
    Log4Qt::LogManager::rootLogger()->addAppender(appender);

    Log4Qt::LogManager::rootLogger()->info("application start");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
