#ifndef SPEAKERCAPTURE_H
#define SPEAKERCAPTURE_H

#include <Windows.h>

#include <QFile>

class MyAudioSink
{
public:
    struct WAVFILEHEADER
    {
        // RIFF 头
        char RiffName[4];
        unsigned long nRiffLength;

        // 数据类型标识符
        char WavName[4];

        // 格式块中的块头
        char FmtName[4];
        unsigned long nFmtLength;

        // 格式块中的块数据
        unsigned short nAudioFormat;
        unsigned short nChannleNumber;
        unsigned long nSampleRate;
        unsigned long nBytesPerSecond;
        unsigned short nBytesPerSample;
        unsigned short nBitsPerSample;

        // 数据块中的块头
        char    DATANAME[4];
        unsigned long   nDataLength;
    };

public:
    MyAudioSink();

    HRESULT SetFormat(WAVEFORMATEX *fomate);
    HRESULT CopyData(BYTE *data, UINT32 length, BOOL *doen);

private:
    int m_channels;
    int m_bitsPerSample;
    int m_samplesPerSec;
    int m_blockAlign;

    QFile tempFile;
};

HRESULT RecordAudioStream(MyAudioSink *pMySink);

#endif // SPEAKERCAPTURE_H
