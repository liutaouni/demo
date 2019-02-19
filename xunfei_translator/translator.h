#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QObject>
#include <QAudioInput>
#include <QIODevice>
#include <QWebSocket>
#include <QFile>

class Translator : public QObject
{
    Q_OBJECT
public:
    explicit Translator(QObject *parent = 0);

signals:
    void sigTranslateResult(const QString &content, bool isEndle);

private slots:
    void slotConnectXunfei();
    void slotReadData();
    void on_connected();
    void on_disconnected();
    void on_textMessageReceived(const QString &message);

private:
    QString getCommonParam();
    QString hmacSha1(QByteArray key, QByteArray baseString);

private:
    QAudioInput *m_audioInput = nullptr;
    QIODevice *m_pcmData = nullptr;
    QWebSocket *m_webSockect = nullptr;
    bool m_isConnected = false;

    QFile *m_log = nullptr;
};

#endif // TRANSLATOR_H
