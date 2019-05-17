#include "speakercapture.h"

#include <QDebug>
#include <QTime>
#include <QByteArray>

#include <combaseapi.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

//-----------------------------------------------------------
// Record an audio stream from the default audio capture
// device. The RecordAudioStream function allocates a shared
// buffer big enough to hold one second of PCM audio data.
// The function uses this buffer to stream data from the
// capture device. The main loop runs every 1/2 second.
//-----------------------------------------------------------

MyAudioSink::MyAudioSink()
{
    tempFile.setFileName("C:/Users/YDKC-20180912-1/Desktop/temp.pcm");
    tempFile.open(QIODevice::ReadWrite);
}

HRESULT MyAudioSink::SetFormat(WAVEFORMATEX *fomate)
{
    m_channels = fomate->nChannels;
    m_bitsPerSample = fomate->wBitsPerSample;
    m_samplesPerSec = fomate->nSamplesPerSec;
    m_blockAlign = fomate->nBlockAlign;
    qDebug() << "======== sample size:" << fomate->wBitsPerSample << "sample rate:" << fomate->nSamplesPerSec << "channel:" << fomate->nChannels;
    return 0;
}

HRESULT MyAudioSink::CopyData(BYTE *data, UINT32 length, BOOL *doen)
{
    static int i = 0;
    i++;

    for(int i = 0, index = 0; i < INT_MAX && index < length * m_blockAlign; i++, index += m_blockAlign)
    {
        if(i % 3 == 0)
        {
            tempFile.write((char*)(data+index), m_blockAlign/2);
        }
    }

    if(i == 1500)
    {
        *doen = true;
        tempFile.seek(0);


        // 开始设置WAV的文件头
        // 这里具体的数据代表什么含义请看上一篇文章（Qt之WAV文件解析）中对wav文件头的介绍
        WAVFILEHEADER WavFileHeader;
        qstrcpy(WavFileHeader.RiffName, "RIFF");
        qstrcpy(WavFileHeader.WavName, "WAVE");
        qstrcpy(WavFileHeader.FmtName, "fmt ");
        qstrcpy(WavFileHeader.DATANAME, "data");

        // 表示 FMT块 的长度
        WavFileHeader.nFmtLength = 16;
        // 表示 按照PCM 编码;
        WavFileHeader.nAudioFormat = 1;
        // 声道数目;
        WavFileHeader.nChannleNumber = m_channels/2;
        // 采样频率;
        WavFileHeader.nSampleRate = m_samplesPerSec/3;

        // 每次采样得到的样本数据位数;
        WavFileHeader.nBitsPerSample = m_bitsPerSample;
        // nBytesPerSample 和 nBytesPerSecond这两个值通过设置的参数计算得到;
        // 数据块对齐单位(每个采样需要的字节数 = 通道数 × 每次采样得到的样本数据位数 / 8 )
        WavFileHeader.nBytesPerSample = WavFileHeader.nChannleNumber*WavFileHeader.nBitsPerSample/8;
        // 波形数据传输速率
        // (每秒平均字节数 = 采样频率 × 通道数 × 每次采样得到的样本数据位数 / 8  = 采样频率 × 每个采样需要的字节数 )
        WavFileHeader.nBytesPerSecond = WavFileHeader.nSampleRate*WavFileHeader.nChannleNumber*WavFileHeader.nBitsPerSample/8;

        int nSize = sizeof(WavFileHeader);
        qint64 nFileLen = tempFile.bytesAvailable();

        WavFileHeader.nRiffLength = nFileLen - 8 + nSize;
        WavFileHeader.nDataLength = nFileLen;

        QFile f("C:/Users/YDKC-20180912-1/Desktop/temp.wav");
        f.open(QFile::WriteOnly);
        // 先将wav文件头信息写入，再将音频数据写入;
        f.write((char *)&WavFileHeader, nSize);
        f.write(tempFile.readAll());
        f.close();

        tempFile.close();
    }

    return 0;
}

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

HRESULT RecordAudioStream(MyAudioSink *pMySink)
{
    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    IAudioClient *pAudioClient = NULL;
    IAudioCaptureClient *pCaptureClient = NULL;
    WAVEFORMATEX *pwfx = NULL;
    UINT32 packetLength = 0;
    BOOL bDone = FALSE;
    BYTE *pData;
    DWORD flags;

    hr = CoCreateInstance(
           CLSID_MMDeviceEnumerator, NULL,
           CLSCTX_ALL, IID_IMMDeviceEnumerator,
           (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

    hr = pEnumerator->GetDefaultAudioEndpoint(
                        eRender, eConsole, &pDevice);
    EXIT_ON_ERROR(hr)

    hr = pDevice->Activate(
                    IID_IAudioClient, CLSCTX_ALL,
                    NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->GetMixFormat(&pwfx);
    EXIT_ON_ERROR(hr)

    switch (pwfx->wFormatTag) {
        case WAVE_FORMAT_IEEE_FLOAT:
            pwfx->wFormatTag = WAVE_FORMAT_PCM;
            pwfx->wBitsPerSample = 16;
            pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
            pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
            break;

        case WAVE_FORMAT_EXTENSIBLE:
            {
                // naked scope for case-local variable
                PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
                if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat)) {
                    pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
                    pEx->Samples.wValidBitsPerSample = 16;
                    pwfx->wBitsPerSample = 16;
                    pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
                    pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
                } else {
                    printf("Don't know how to coerce mix format to int-16\n");
                    EXIT_ON_ERROR(-1)
                }
            }
            break;

        default:
            printf("Don't know how to coerce WAVEFORMATEX with wFormatTag = 0xx to int-16\n", pwfx->wFormatTag);
            EXIT_ON_ERROR(-1)
    }

    hr = pAudioClient->Initialize(
                         AUDCLNT_SHAREMODE_SHARED,
                         AUDCLNT_STREAMFLAGS_LOOPBACK,
                         hnsRequestedDuration,
                         0,
                         pwfx,
                         NULL);
    EXIT_ON_ERROR(hr)

    // Get the size of the allocated buffer.
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->GetService(
                         IID_IAudioCaptureClient,
                         (void**)&pCaptureClient);
    EXIT_ON_ERROR(hr)

    // Notify the audio sink which format to use.
    hr = pMySink->SetFormat(pwfx);
    EXIT_ON_ERROR(hr)

    // Calculate the actual duration of the allocated buffer.
    hnsActualDuration = (double)REFTIMES_PER_SEC *
                     bufferFrameCount / pwfx->nSamplesPerSec;

    hr = pAudioClient->Start();  // Start recording.
    EXIT_ON_ERROR(hr)

    // Each loop fills about half of the shared buffer.
    while (bDone == FALSE)
    {
        // Sleep for half the buffer duration.
        // Sleep(hnsActualDuration/REFTIMES_PER_MILLISEC/2);
        Sleep(40);

        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        EXIT_ON_ERROR(hr)

        while (packetLength != 0)
        {
            // Get the available data in the shared buffer.
            hr = pCaptureClient->GetBuffer(
                                   &pData,
                                   &numFramesAvailable,
                                   &flags, NULL, NULL);
            EXIT_ON_ERROR(hr)

            // if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            // {
            //     pData = NULL;  // Tell CopyData to write silence.
            // }

            // Copy the available capture data to the audio sink.
            hr = pMySink->CopyData(
                              pData, numFramesAvailable, &bDone);
            EXIT_ON_ERROR(hr)

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            EXIT_ON_ERROR(hr)

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            EXIT_ON_ERROR(hr)
        }
    }

    hr = pAudioClient->Stop();  // Stop recording.
    EXIT_ON_ERROR(hr)

Exit:
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pDevice)
    SAFE_RELEASE(pAudioClient)
    SAFE_RELEASE(pCaptureClient)

    return hr;
}
