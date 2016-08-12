// BMD split

#pragma once

#include <mutex>
#include "DeckLinkAPI.h"

class BMDSplit: public IDeckLinkInputCallback
{
public:
    BMDSplit();
    virtual ~BMDSplit() {}

    bool run(int32_t videoMode);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) { return E_NOINTERFACE; }
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE  Release(void);
    virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
    virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);

protected:
    ULONG refCount;
    std::mutex dataMutex;

    IDeckLink* deckLink;
    IDeckLinkInput* deckLinkInput;
    IDeckLinkDisplayModeIterator* displayModeIterator;
    IDeckLinkDisplayMode* displayMode;
    IDeckLinkConfiguration* deckLinkConfiguration;
};
