#include "widget.h"
#include "ui_widget.h"

#include "speakercapture.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    MyAudioSink my;
    RecordAudioStream(&my);
}

Widget::~Widget()
{
    delete ui;
}
