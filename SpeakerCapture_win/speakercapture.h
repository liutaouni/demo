#ifndef SPEAKERCAPTURE_H
#define SPEAKERCAPTURE_H

#include <Windows.h>

class MyAudioSink
{
public:
    HRESULT SetFormat(WAVEFORMATEX *fomate);
    HRESULT CopyData(BYTE *data, UINT32 length, BOOL *doen);
};

HRESULT RecordAudioStream(MyAudioSink *pMySink);

#endif // SPEAKERCAPTURE_H
