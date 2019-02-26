#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

#include "widget.h"
#include "ui_widget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <QStandardPaths>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->savePathLineEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
}

Widget::~Widget()
{
    if(m_pro)
    {
        m_pro->close();
        m_pro->waitForFinished();
        m_pro->deleteLater();
        m_pro = NULL;
    }

    delete ui;
}

void Widget::on_saveBtn_clicked()
{
    if(m_pro)
    {
        QProcess::ProcessState state = m_pro->state();
        if(state != QProcess::NotRunning)
        {
            QMessageBox::warning(this, tr("提示"), tr("正在提取音频文件..."));
            return;
        }
    }

    if(m_tran)
    {
        if(m_tran->getIsWorking())
        {
            QMessageBox::warning(this, tr("提示"), tr("正在将音频文件转换为文字..."));
            return;
        }
    }

    QString dirPath = QFileDialog::getExistingDirectory(this, tr("保存到"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    if(!dirPath.isEmpty())
    {
        ui->savePathLineEdit->setText(dirPath);
    }
}

void Widget::on_openBtn_clicked()
{
    if(m_pro)
    {
        QProcess::ProcessState state = m_pro->state();
        if(state != QProcess::NotRunning)
        {
            QMessageBox::warning(this, tr("提示"), tr("正在提取音频文件..."));
            return;
        }
    }

    if(m_tran)
    {
        if(m_tran->getIsWorking())
        {
            QMessageBox::warning(this, tr("提示"), tr("正在将音频文件转换为文字..."));
            return;
        }
    }

    QString filePath = QFileDialog::getOpenFileName(NULL, tr("打开"), QStandardPaths::displayName(QStandardPaths::DocumentsLocation));
    if(!filePath.isEmpty())
    {
        ui->videoPathLineEdit->setText(filePath);
        ui->textBrowser->setText("");

        if(!m_pro)
        {
            m_pro = new QProcess();
            connect(m_pro, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                  this, &Widget::slotProFinished);
            connect(m_pro, &QProcess::errorOccurred, this, &Widget::slotProErrorOccurred);
        }

        QString tempFilePath = QDir::currentPath() + "/temp.mp3";
        QFile tempfile(tempFilePath);
        if(tempfile.exists() && !tempfile.remove())
        {
            QMessageBox::warning(this, tr("提示"), tr("无法删除音频文件 temp.mp3"));
            return;
        }

        QStringList args;
        args << "-i";
        args << QDir::toNativeSeparators(filePath);
        args << "-f";
        args << "mp3";
        args << QDir::toNativeSeparators(tempFilePath);
        m_pro->start("ffmpeg.exe", args);

        ui->statusLabel->setText(tr("正在提取音频信息..."));
    }
}

void Widget::slotProFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    ui->statusLabel->setText("");

    if(exitStatus == QProcess::CrashExit)
    {
        QMessageBox::warning(this, tr("提示"), tr("提取音频文件程序 崩溃"));
    }
    else
    {
        if(exitCode == 0)
        {
            if(!m_tran)
            {
                m_tran = new Translator();
                connect(m_tran, &Translator::sigStatusChange, this, &Widget::slotStatusChange);
                connect(m_tran, &Translator::sigTranslateError, this, &Widget::slotTranslateError);
                connect(m_tran, &Translator::sigTranslateFinished, this, &Widget::slotTranslateFinished);
            }

            QString tempFilePath = QDir::currentPath() + "/temp.mp3";
            QFile tempFile(tempFilePath);
            if(tempFile.exists() && tempFile.size())
            {
                if(tempFile.open(QIODevice::ReadOnly))
                {
                    tempFile.close();
                    m_tran->start(tempFilePath);
                }
                else
                {
                    QMessageBox::warning(this, tr("提示"), tr("音频文件 打开失败"));
                }
            }
            else
            {
                QMessageBox::warning(this, tr("提示"), tr("音频文件 不存在或大小为0"));
            }
        }
        else
        {
            QMessageBox::warning(this, tr("提示"), tr("提取音频文件程序 exit code:%1").arg(exitCode));
        }
    }
}

void Widget::slotProErrorOccurred(QProcess::ProcessError error)
{
    ui->statusLabel->setText("");
    QMessageBox::warning(this, tr("提示"), tr("提取音频文件程序 error code:%1").arg((int)error));
}

void Widget::slotStatusChange(const QString &status)
{
    ui->statusLabel->setText(status);
}

void Widget::slotTranslateError(const QString &errStr)
{
    ui->statusLabel->setText("");
    QMessageBox::warning(this, tr("提示"), errStr);
}

void Widget::slotTranslateFinished(const QStringList &content)
{
    ui->statusLabel->setText("");
    ui->textBrowser->setText(content.join("\n"));

    if(!QDir(ui->savePathLineEdit->text()).exists())
    {
        QDir().mkpath(ui->savePathLineEdit->text());
    }

    QString fileName = QFileInfo(ui->videoPathLineEdit->text()).completeBaseName();
    QString filePath = ui->savePathLineEdit->text() + "/" + fileName + ".txt";
    int index = 1;
    while(QFile(filePath).exists())
    {
        filePath = ui->savePathLineEdit->text() + "/" + fileName + "(" + QString::number(index++) + ").txt";
    }
    QFile txtFile(filePath);
    if (txtFile.open(QFile::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&txtFile);
        for(int i = 0; i < content.length(); i++)
        {
            out << content.at(i) << endl;
        }
        txtFile.close();
        QMessageBox::warning(this, tr("提示"), tr("已保存到 %1").arg(filePath));
    }
}


