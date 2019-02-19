#ifndef ZHTOEN_H
#define ZHTOEN_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QScriptEngine>

class zhToEn : public QObject
{
    Q_OBJECT
public:
    explicit zhToEn(QObject *parent = 0);

signals:
    void sigFinished(const QString &enStr, bool isEndle);

public slots:
    void slotTranslateResult(const QString &content, bool isEndle);

private slots:
    void slotZhToEnFinished();

private:
    QNetworkAccessManager m_netMngr;
    QScriptEngine m_jsEngine;
};

#endif // ZHTOEN_H
