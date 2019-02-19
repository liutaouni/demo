#include "zhtoen.h"

#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>

zhToEn::zhToEn(QObject *parent) : QObject(parent)
{
    QFile scriptFile(":/res/google.js");
    if (scriptFile.open(QIODevice::ReadOnly))
    {
        QTextStream out(&scriptFile);
        QString contents = out.readAll();
        scriptFile.close();

        m_jsEngine.evaluate(contents);
    }
}

void zhToEn::slotTranslateResult(const QString &content, bool isEndle)
{
    if(isEndle)
    {
        QScriptValue func = m_jsEngine.globalObject().property("token");
        QScriptValueList args;
        args << QScriptValue(content);
        QString token = func.call(QScriptValue(), args).toString();

        QString googleUrl = "http://translate.google.cn/translate_a/single";
        googleUrl += "?client=webapp";
        googleUrl += "&sl=zh-CN";
        googleUrl += "&tl=en";
        googleUrl += "&hl=zh-CN";
        googleUrl += "&dt=at";
        googleUrl += "&dt=bd";
        googleUrl += "&dt=ex";
        googleUrl += "&dt=ld";
        googleUrl += "&dt=md";
        googleUrl += "&dt=qca";
        googleUrl += "&dt=rw";
        googleUrl += "&dt=rm";
        googleUrl += "&dt=ss";
        googleUrl += "&dt=t";
        googleUrl += "&otf=1";
        googleUrl += "&ssel=3";
        googleUrl += "&tsel=0";
        googleUrl += "&kc=4";
        googleUrl += "&tk=" + token;
        googleUrl += "&q=" + QUrl::toPercentEncoding(content);

        QNetworkRequest req(googleUrl);
        QNetworkReply *reply = m_netMngr.get(req);
        connect(reply, &QNetworkReply::finished, this, &zhToEn::slotZhToEnFinished);
    }
}

void zhToEn::slotZhToEnFinished()
{
    QNetworkReply *replay = qobject_cast<QNetworkReply *>(sender());

    if (replay)
    {
        QString result;
        QString srcText;

        QByteArray bytes = replay->readAll();
        replay->deleteLater();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(bytes);
        QJsonArray jsonArr = jsonDoc.array();
        if(jsonArr.size())
        {
            QJsonArray innerArray = jsonArr.first().toArray();
            if(innerArray.size())
            {
                QJsonArray finalArray = innerArray.first().toArray();
                if(finalArray.size() > 1 && finalArray.first().toString().size())
                {
                    result = finalArray[0].toString();
                    srcText = finalArray[1].toString();
                    emit sigFinished(result, true);
                }
            }
        }
    }
}
