// BMD split

#pragma once

#include <mutex>
#include "Network.h"
#include "DeckLinkAPI.h"

class BMDSplit: public IDeckLinkInputCallback
{
public:
    BMDSplit(cppsocket::Network& pNetwork);
    virtual ~BMDSplit() {}

    bool run(int32_t videoMode);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) { return E_NOINTERFACE; }
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE  Release(void);
    virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
    virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);

protected:
    cppsocket::Network& network;

    ULONG refCount;
    std::mutex dataMutex;

    IDeckLink* deckLink;
    IDeckLinkInput* deckLinkInput;
    IDeckLinkDisplayModeIterator* displayModeIterator;
    IDeckLinkDisplayMode* displayMode;
    IDeckLinkConfiguration* deckLinkConfiguration;
};
