#include "translator.h"

#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTime>

Translator::Translator(QObject *parent) : QObject(parent)
{
    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        format = info.nearestFormat(format);
    }

    m_audioInput = new QAudioInput(format);
    m_pcmData = m_audioInput->start();
    connect(m_pcmData, &QIODevice::readyRead, this, &Translator::slotReadData);

    m_webSockect = new QWebSocket();
    connect(m_webSockect,&QWebSocket::connected,this,&Translator::on_connected);
    connect(m_webSockect,&QWebSocket::disconnected,this,&Translator::on_disconnected);
    connect(m_webSockect,&QWebSocket::textMessageReceived,this,&Translator::on_textMessageReceived);

    //m_log = new QFile("log.txt");
    //m_log->open(QIODevice::WriteOnly | QIODevice::Text);

    slotConnectXunfei();
}

void Translator::slotConnectXunfei()
{
    if(m_log){
        m_log->write("open socket");
    }
    qDebug() << "==============open==============";
    m_webSockect->open(QUrl("ws://rtasr.xfyun.cn/v1/ws?"+getCommonParam()));
}

void Translator::slotReadData()
{
    QByteArray data = m_pcmData->readAll();
    if(m_log){
        m_log->write(QString::number(data.length()).toLocal8Bit());
    }
    if(m_isConnected && data.length()){
        m_webSockect->sendBinaryMessage(data);
    }
}

void Translator::on_connected()
{
    m_isConnected = true;
    if(m_log){
        m_log->write("==============connected=============");
    }
    qDebug() << "==============connected==============";
}

void Translator::on_disconnected()
{
    m_isConnected = false;
    if(m_log){
        m_log->write("==============disconnected=============");
    }
    qDebug() << "==============disconnected==============";
    QTimer::singleShot(100, this, &Translator::slotConnectXunfei);
}

void Translator::on_textMessageReceived(const QString &message)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();

    if(jsonObj["action"].toString() == "result" && jsonObj["code"].toString() == "0")
    {
        QString dataJsonStr = jsonObj["data"].toString();
        QJsonDocument datajsonDoc = QJsonDocument::fromJson(dataJsonStr.toUtf8());

        QJsonObject dataObj = datajsonDoc.object();
        QJsonObject cnObj = dataObj["cn"].toObject();
        QJsonObject stObj = cnObj["st"].toObject();
        QJsonArray rtArr = stObj["rt"].toArray();
        QString content;
        if(rtArr.count())
        {
            QJsonArray wsArr = rtArr.first().toObject()["ws"].toArray();
            for(int i = 0; i < wsArr.count(); i++)
            {
                QJsonArray cwArr = wsArr[i].toObject()["cw"].toArray();
                if(cwArr.count())
                {
                    QString text = cwArr.first().toObject()["w"].toString();
                    QString wp = cwArr.first().toObject()["wp"].toString();
                    if(content.isEmpty() && wp == "p")
                    {
                        //忽略第一个标点符号
                    }
                    else
                    {
                        content += text;
                    }
                }
            }
        }
        QString typeStr = stObj["type"].toString();

        if(content.length())
        {
            emit sigTranslateResult(content, typeStr == "0");
        }
    }
    else
    {
        if(m_log){
            m_log->write(message.toLocal8Bit());
        }
        qDebug() << "==============" << message;
    }
}

QString Translator::hmacSha1(QByteArray key, QByteArray baseString)
{
    int blockSize = 64; // HMAC-SHA-1 block size, defined in SHA-1 standard
    if (key.length() > blockSize) { // if key is longer than block size (64), reduce key length with SHA-1 compression
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
    }
    QByteArray innerPadding(blockSize, char(0x36)); // initialize inner padding with char"6"
    QByteArray outerPadding(blockSize, char(0x5c)); // initialize outer padding with char"/"
    // ascii characters 0x36 ("6") and 0x5c ("/") are selected because they have large
    // Hamming distance (http://en.wikipedia.org/wiki/Hamming_distance)
    for (int i = 0; i < key.length(); i++) {
        innerPadding[i] = innerPadding[i] ^ key.at(i); // XOR operation between every byte in key and innerpadding, of key length
        outerPadding[i] = outerPadding[i] ^ key.at(i); // XOR operation between every byte in key and outerpadding, of key length
    }
    // result = hash ( outerPadding CONCAT hash ( innerPadding CONCAT baseString ) ).toBase64
    QByteArray total = outerPadding;
    QByteArray part = innerPadding;
    part.append(baseString);
    total.append(QCryptographicHash::hash(part, QCryptographicHash::Sha1));
    QByteArray hashed = QCryptographicHash::hash(total, QCryptographicHash::Sha1);
    return hashed.toBase64();
}

QString Translator::getCommonParam()
{
    QString appId = "5c6b9bde";
    QString ts = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()/1000);
    QByteArray apiKey = "239da699484c1b49cd097862fcef7bd4";

    QString baseString = appId + ts;
    QByteArray md5 = QCryptographicHash::hash(baseString.toUtf8(),QCryptographicHash::Md5).toHex();
    QString signa = hmacSha1(apiKey, md5);

    QString param;
    param += "appid=" + appId;
    param += "&ts=" + ts;
    param += "&signa=" + signa;

    return param;
}
