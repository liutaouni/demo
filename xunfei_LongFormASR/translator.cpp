#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

#include "translator.h"

#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QProcess>
#include <QDir>
#include <QHttpPart>

void SliceIdGenerator::reset()
{
    ch = "";
}

QString SliceIdGenerator::getNextSliceId()
{
    if(ch.isEmpty())
    {
        ch = "aaaaaaaaaa";
    }
    else
    {
        int j = ch.length() - 1;
        while (j >= 0)
        {
            QChar cj = ch.at(j);
            if (cj != 'z')
            {
                ch = ch.left(j) + QChar(cj.unicode()+1) + ch.right(ch.length()-j-1);
                break;
            }
            else
            {
                j = j -1;
            }
        }
    }

    return ch;
}

QString SliceIdGenerator::getCurSliceId()
{
    return ch;
}

Translator::Translator(QObject *parent) : QObject(parent)
{

}

bool Translator::getIsWorking()
{
    return m_isWorking;
}

void Translator::start(const QString &filePath)
{
    m_isWorking = true;
    m_soundFilePath = filePath;
    if(m_soundFile.isOpen())
    {
        m_soundFile.close();
    }
    m_soundFile.setFileName(m_soundFilePath);

    if(m_soundFile.size() % m_sliceSize){
        m_sliceNum = m_soundFile.size() / m_sliceSize + 1;
    }else{
        m_sliceNum = m_soundFile.size() / m_sliceSize;
    }

    QNetworkRequest request;
    request.setUrl(QUrl("http://raasr.xfyun.cn/api/prepare"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");

    QByteArray args = getCommonParam();
    args += "&file_len=" + QByteArray::number(m_soundFile.size());
    args += "&file_name=" + QUrl::toPercentEncoding(QFileInfo(filePath).fileName());
    args += "&slice_num=" + QByteArray::number(m_sliceNum);

    QNetworkReply *reply = m_netMngr.post(request, args);
    connect(reply, &QNetworkReply::finished, this, &Translator::slotPrepareFinished);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
              this, SLOT(slotPrepareError(QNetworkReply::NetworkError)));

    emit sigStatusChange(tr("prepare..."));
}

void Translator::slotPrepareError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(reply)
    {
        reply->deleteLater();
        reply = NULL;
    }

    emit sigTranslateError(tr("网络错误，error code:%1").arg((int)error));
    m_isWorking = false;
}

void Translator::slotPrepareFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(reply)
    {
        QByteArray result = reply->readAll();
        reply->deleteLater();
        reply = NULL;

        QJsonDocument jsonDoc = QJsonDocument::fromJson(result);
        QJsonObject jsonObj = jsonDoc.object();
        if(jsonObj["err_no"].toInt())
        {
            emit sigTranslateError(jsonObj["failed"].toString());
            m_isWorking = false;
        }
        else
        {
            if(m_soundFile.open(QIODevice::ReadOnly))
            {
                m_taskId = jsonObj["data"].toString();
                m_curSliceNum = 0;
                m_sliceIdGenrator.reset();

                sliceUpload();
            }
            else
            {
                emit sigTranslateError(tr("音频文件打开失败"));
                m_isWorking = false;
            }
        }
    }
}

void Translator::sliceUpload(bool isRetry)
{
    QByteArray args = getCommonParam();
    args += "&task_id=" + m_taskId;
    if(isRetry){
        args += "&slice_id=" + m_sliceIdGenrator.getCurSliceId().toLatin1();
    }else{
        args += "&slice_id=" + m_sliceIdGenrator.getNextSliceId().toLatin1();
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart audioPart;
    audioPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    audioPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(tr("form-data; name=\"content\";filename=\"%1\"").arg(QFileInfo(m_soundFilePath).fileName())));
    if(isRetry){
        audioPart.setBody(m_curSliceData);
    }else{
        m_curSliceData = m_soundFile.read(m_sliceSize);
        audioPart.setBody(m_curSliceData);
    }
    multiPart->append(audioPart);

    QNetworkRequest request;
    request.setUrl(QUrl("http://raasr.xfyun.cn/api/upload?" + args));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary="+multiPart->boundary());

    QNetworkReply *reply = m_netMngr.post(request, multiPart);
    multiPart->setParent(reply);
    connect(reply, &QNetworkReply::finished, this, &Translator::slotUplaodFinished);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
              this, SLOT(slotUploadError(QNetworkReply::NetworkError)));

    if(isRetry){
        emit sigStatusChange(tr("上传文件，第 %1 片(重试)...").arg(m_curSliceNum));
    }else{
        emit sigStatusChange(tr("上传文件，第 %1 片...").arg(m_curSliceNum));
    }
}

void Translator::slotSliceUploadRetry()
{
    sliceUpload(true);
}

void Translator::slotUploadError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(reply)
    {
        reply->deleteLater();
        reply = NULL;
    }

    QTimer::singleShot(10*1000, this, &Translator::slotSliceUploadRetry);
}

void Translator::slotUplaodFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(reply)
    {
        QByteArray result = reply->readAll();
        reply->deleteLater();
        reply = NULL;

        QJsonDocument jsonDoc = QJsonDocument::fromJson(result);
        QJsonObject jsonObj = jsonDoc.object();
        if(jsonObj["err_no"].toInt())
        {
            emit sigTranslateError(jsonObj["failed"].toString());
            m_isWorking = false;
        }
        else
        {
            m_curSliceNum += 1;
            if(m_curSliceNum < m_sliceNum)
            {
                sliceUpload();
            }
            else
            {
                sliceMerge();
            }
        }
    }
}

void Translator::sliceMerge(bool isRetry)
{
    QNetworkRequest request;
    request.setUrl(QUrl("http://raasr.xfyun.cn/api/merge"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");

    QByteArray args = getCommonParam();
    args += "&task_id=" + m_taskId;

    QNetworkReply *reply = m_netMngr.post(request, args);
    connect(reply, &QNetworkReply::finished, this, &Translator::slotMergeFinished);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
              this, SLOT(slotMergeError(QNetworkReply::NetworkError)));

    if(isRetry){
        emit sigStatusChange(tr("merge(重试)..."));
    }else{
        emit sigStatusChange(tr("merge..."));
    }
}

void Translator::slotSliceMergeRetry()
{
    sliceMerge(true);
}

void Translator::slotMergeError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(reply)
    {
        reply->deleteLater();
        reply = NULL;
    }

    QTimer::singleShot(10*1000, this, &Translator::slotSliceMergeRetry);
}

void Translator::slotMergeFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(reply)
    {
        QByteArray result = reply->readAll();
        reply->deleteLater();
        reply = NULL;

        QJsonDocument jsonDoc = QJsonDocument::fromJson(result);
        QJsonObject jsonObj = jsonDoc.object();
        if(jsonObj["err_no"].toInt())
        {
            emit sigTranslateError(jsonObj["failed"].toString());
            m_isWorking = false;
        }
        else
        {
            emit sigStatusChange(tr("进行中..."));

            QProcess pro;
            QStringList proArgs;
            proArgs << "-v";
            proArgs << "quiet";
            proArgs << "-print_format";
            proArgs << "json";
            proArgs << "-show_format";
            proArgs << QDir::toNativeSeparators(m_soundFilePath);
            pro.start("ffprobe.exe", proArgs);
            pro.waitForFinished();
            QByteArray mediaInfo = pro.readAll();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(mediaInfo);
            QJsonObject jsonObj = jsonDoc.object();
            double duration_sec = jsonObj["format"].toObject()["duration"].toString().toDouble();

            if(duration_sec > 1*60*60){
                m_checkProgessInterval = 10*60*1000;
            }else if(duration_sec > 30*60){
                m_checkProgessInterval = 5*60*1000;
            }else if(duration_sec > 10*60){
                m_checkProgessInterval = 2*60*1000;
            }else{
                m_checkProgessInterval = 1*60*1000;
            }

            QTimer::singleShot(m_checkProgessInterval, this, &Translator::slotCheckProgress);
        }
    }
}

void Translator::slotCheckProgress()
{
    QNetworkRequest request;
    request.setUrl(QUrl("http://raasr.xfyun.cn/api/getProgress"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");

    QByteArray args = getCommonParam();
    args += "&task_id=" + m_taskId;

    QNetworkReply *reply = m_netMngr.post(request, args);
    connect(reply, &QNetworkReply::finished, this, &Translator::slotProgressFinished);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
              this, SLOT(slotProgressError(QNetworkReply::NetworkError)));

    emit sigStatusChange(tr("进行中..."));
}

void Translator::slotProgressError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(reply)
    {
        reply->deleteLater();
        reply = NULL;
    }

    QTimer::singleShot(10*1000, this, &Translator::slotCheckProgress);
}

void Translator::slotProgressFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(reply)
    {
        QByteArray result = reply->readAll();
        reply->deleteLater();
        reply = NULL;

        QJsonDocument jsonDoc = QJsonDocument::fromJson(result);
        QJsonObject jsonObj = jsonDoc.object();
        if(jsonObj["err_no"].toInt())
        {
            emit sigTranslateError(jsonObj["failed"].toString());
            m_isWorking = false;
        }
        else
        {
            QString dataStr = jsonObj["data"].toString();
            QJsonDocument dataJsonDoc = QJsonDocument::fromJson(dataStr.toUtf8());
            QJsonObject dataJsonObj = dataJsonDoc.object();
            int status = dataJsonObj["status"].toInt();
            if(status == 9)
            {
                requestResult();
            }
            else
            {
                QTimer::singleShot(m_checkProgessInterval, this, &Translator::slotCheckProgress);
            }
        }
    }
}

void Translator::requestResult(bool isRetry)
{
    QNetworkRequest request;
    request.setUrl(QUrl("http://raasr.xfyun.cn/api/getResult"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");

    QByteArray args = getCommonParam();
    args += "&task_id=" + m_taskId;

    QNetworkReply *reply = m_netMngr.post(request, args);
    connect(reply, &QNetworkReply::finished, this, &Translator::slotResultFinished);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
              this, SLOT(slotResultError(QNetworkReply::NetworkError)));

    if(isRetry){
        emit sigStatusChange(tr("获取结果(重试)..."));
    }else{
        emit sigStatusChange(tr("获取结果..."));
    }
}

void Translator::slotRequestResultRetry()
{
    requestResult(true);
}

void Translator::slotResultError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(reply)
    {
        reply->deleteLater();
        reply = NULL;
    }

    QTimer::singleShot(10*1000, this, &Translator::slotRequestResultRetry);
}

void Translator::slotResultFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(reply)
    {
        QByteArray result = reply->readAll();
        reply->deleteLater();
        reply = NULL;

        QJsonDocument jsonDoc = QJsonDocument::fromJson(result);
        QJsonObject jsonObj = jsonDoc.object();
        if(jsonObj["err_no"].toInt())
        {
            emit sigTranslateError(jsonObj["failed"].toString());
            m_isWorking = false;
        }
        else
        {
            QString dataStr = jsonObj["data"].toString();
            QJsonDocument dataJsonDoc = QJsonDocument::fromJson(dataStr.toUtf8());
            QJsonArray dataArr = dataJsonDoc.array();

            QStringList content;
            for(int i = 0; i < dataArr.size(); i++)
            {
                content.append(dataArr[i].toObject()["onebest"].toString());
            }

            emit sigTranslateFinished(content);
            m_isWorking = false;
        }
    }
}

QByteArray Translator::hmacSha1(QByteArray key, QByteArray baseString)
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

QByteArray Translator::getCommonParam()
{
    QByteArray appId = "5c6b9bde";
    QByteArray ts = QByteArray::number(QDateTime::currentDateTime().toMSecsSinceEpoch()/1000);
    QByteArray apiKey = "bf29a7f00e8be35b448dfc284e99be55";

    QByteArray baseString = appId + ts;
    QByteArray md5 = QCryptographicHash::hash(baseString,QCryptographicHash::Md5).toHex();
    QByteArray signa = hmacSha1(apiKey, md5);

    QByteArray param;
    param += "&app_id=" + appId;
    param += "&ts=" + ts;
    param += "&signa=" + QUrl::toPercentEncoding(signa);

    return param;
}
