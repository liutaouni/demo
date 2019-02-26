#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

class SliceIdGenerator
{
public:
    void reset();
    QString getNextSliceId();
    QString getCurSliceId();

private:
    QString ch;
};

class Translator : public QObject
{
    Q_OBJECT
public:
    explicit Translator(QObject *parent = 0);

    bool getIsWorking();
    void start(const QString &filePath);

signals:
    void sigStatusChange(const QString &status);
    void sigTranslateError(const QString &errStr);
    void sigTranslateFinished(const QStringList &content);

private slots:
    void slotPrepareError(QNetworkReply::NetworkError);
    void slotPrepareFinished();
    void slotUploadError(QNetworkReply::NetworkError);
    void slotUplaodFinished();
    void slotMergeError(QNetworkReply::NetworkError);
    void slotMergeFinished();
    void slotProgressError(QNetworkReply::NetworkError);
    void slotProgressFinished();
    void slotResultError(QNetworkReply::NetworkError);
    void slotResultFinished();

    void slotSliceUploadRetry();
    void slotSliceMergeRetry();
    void slotCheckProgress();
    void slotRequestResultRetry();

private:
    QByteArray getCommonParam();
    QByteArray hmacSha1(QByteArray key, QByteArray baseString);

    void sliceUpload(bool isRetry=false);
    void sliceMerge(bool isRetry=false);
    void requestResult(bool isRetry=false);

private:
    bool m_isWorking = false;
    QString m_soundFilePath;
    QFile m_soundFile;
    QString m_taskId;
    const qint64 m_sliceSize = 3*1024*1024;
    int m_sliceNum = 0;
    int m_curSliceNum = 0;
    QByteArray m_curSliceData;
    SliceIdGenerator m_sliceIdGenrator;
    int m_checkProgessInterval = 0;

    QNetworkAccessManager m_netMngr;
};

#endif // TRANSLATOR_H
