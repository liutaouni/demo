#include "widget.h"
#include "ui_widget.h"

#include <QDebug>
#include <QPainter>
#include <QTextBlock>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    this->setObjectName("win");
    this->setStyleSheet("QWidget#win{border-image:url(:/res/bg.png); background:rgb(31,7,8);}");
    ui->label->setStyleSheet("background:rgba(42,44,68,128); color:#94feff; font-size:18px; font-weight:bold; padding-top:15px; padding-bottom:15px;");
    ui->textBrowser->setStyleSheet("background:transparent; font-size:18px; color:white;");
    ui->contentWidget->setStyleSheet("QWidget#contentWidget{border:2px solid white; background:rgba(255,255,255,128);}");
    ui->clearBtn->setStyleSheet("border:none;");
    ui->sizeBtn->setStyleSheet("border:none;");

    ui->label->installEventFilter(this);
    ui->sizeBtn->installEventFilter(this);
}

Widget::~Widget()
{
    delete ui;
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->label)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            m_isMousePressed = true;
            m_pressedPos = QCursor::pos();
            m_winPos = this->pos();
        }
        else if(event->type() == QEvent::MouseMove)
        {
            if(m_isMousePressed)
            {
                this->move(m_winPos + QCursor::pos() - m_pressedPos);
            }
        }
        else if(event->type() == QEvent::MouseButtonRelease)
        {
            m_isMousePressed = false;
        }
        else if(event->type() == QEvent::MouseButtonDblClick)
        {
            this->setWindowState(this->windowState() ^ Qt::WindowFullScreen);
        }
    }
    else if(watched == ui->sizeBtn)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            m_isMousePressForResizeWin = true;
            m_pressedPosForResizeWin = QCursor::pos();
            m_winSizeForResizeWin = this->size();
            ui->sizeLabel->setText(QString::number(this->size().width())+"*"+QString::number(this->size().height()));
        }
        else if(event->type() == QEvent::MouseMove)
        {
            if(m_isMousePressForResizeWin)
            {
                this->resize(m_winSizeForResizeWin.width()+QCursor::pos().x()-m_pressedPosForResizeWin.x(),
                             m_winSizeForResizeWin.height()+QCursor::pos().y()-m_pressedPosForResizeWin.y());
                ui->sizeLabel->setText(QString::number(this->size().width())+"*"+QString::number(this->size().height()));
            }
        }
        else if(event->type() == QEvent::MouseButtonRelease)
        {
            m_isMousePressForResizeWin = false;
            ui->sizeLabel->setText("");
        }
    }

    return QWidget::eventFilter(watched, event);
}

void Widget::paintEvent(QPaintEvent *e)
{
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
    QWidget::paintEvent(e);
}

void Widget::slotShowTranslateResult(const QString &content, bool isEndle)
{
    if(m_textList.size())
    {
        if(m_textList.last().second)
        {
            if(m_textList.size() == 10)
            {
                m_textList.takeFirst();
            }

            m_textList.append(QPair<QString, bool>(content, isEndle));
        }
        else
        {
            m_textList.last().first = content;
            m_textList.last().second = isEndle;
        }
    }
    else
    {
        m_textList.append(QPair<QString, bool>(content, isEndle));
    }

    QString textStr;
    for(auto &item : m_textList)
    {
        textStr.append(item.first + "\n");
    }

    ui->textBrowser->setText(textStr);

    QTextDocument *doc =  ui->textBrowser->document();
    QTextCursor textcursor = ui->textBrowser->textCursor();
    for(QTextBlock it = doc->begin(); it !=doc->end();it = it.next())
    {
        QTextBlockFormat tbf = it.blockFormat();
        tbf.setLineHeight(10,QTextBlockFormat::LineDistanceHeight);
        tbf.setAlignment(Qt::AlignHCenter);
        textcursor.setPosition(it.position());
        textcursor.setBlockFormat(tbf);
        ui->textBrowser->setTextCursor(textcursor);
    }
}


void Widget::on_clearBtn_clicked()
{
    m_textList.clear();
    ui->textBrowser->setText("");
}
