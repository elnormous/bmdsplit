//
//  BMD split
//

#pragma once

#include <mutex>
#include "Network.h"
#include "Acceptor.h"
#include "DeckLinkAPI.h"

class BMDSplit: public IDeckLinkInputCallback
{
public:
    BMDSplit(cppsocket::Network& pNetwork, uint16_t pPort);
    virtual ~BMDSplit();

    bool run(int32_t videoMode);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID*) { return E_NOINTERFACE; }
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE  Release();
    virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents events, IDeckLinkDisplayMode* newDisplayMode, BMDDetectedVideoInputFormatFlags flags);
    virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame,
                                                             IDeckLinkAudioInputPacket* audioFrame);

protected:
    void acceptCallback(cppsocket::Socket& client);
    void sendMetaData(cppsocket::Socket& client);

    cppsocket::Network& network;
    uint16_t port;
    cppsocket::Acceptor socket;
    std::vector<cppsocket::Socket> clients;

    ULONG refCount;
    std::mutex dataMutex;

    IDeckLink* deckLink = nullptr;
    IDeckLinkInput* deckLinkInput = nullptr;
    IDeckLinkDisplayModeIterator* displayModeIterator = nullptr;
    IDeckLinkDisplayMode* displayMode = nullptr;
    IDeckLinkConfiguration* deckLinkConfiguration = nullptr;

    BMDDisplayMode selectedDisplayMode = bmdModeNTSC;
    BMDPixelFormat pixelFormat = bmdFormat8BitYUV;
    long width;
    long height;
    BMDTimeValue frameDuration = 0;
    BMDTimeValue timeScale = 0;
    BMDFieldDominance fieldDominance;

    BMDAudioSampleRate audioSampleRate = bmdAudioSampleRate48kHz;
    BMDAudioSampleType audioSampleDepth = bmdAudioSampleType16bitInteger;
    uint32_t audioChannels = 2;
};
