#include "speakercapture.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MyAudioSink sink;
    RecordAudioStream(&sink);

    return a.exec();
}
