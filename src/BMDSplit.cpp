// BMD split

#include <iostream>
#include "BMDSplit.h"

BMDSplit::BMDSplit(cppsocket::Network& pNetwork):
    network(pNetwork)
{
}

bool BMDSplit::run(int32_t videoMode)
{
    IDeckLinkIterator* deckLinkIterator = CreateDeckLinkIteratorInstance();

    if (!deckLinkIterator)
    {
        std::cerr << "This application requires the DeckLink drivers installed\n";
        return false;
    }

    HRESULT result;

    do
    {
        result = deckLinkIterator->Next(&deckLink);
    }
    while (result != S_OK);

    if (result != S_OK)
    {
        std::cerr << "No DeckLink PCI cards found\n";
        return false;
    }

    if (deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&deckLinkInput) != S_OK)
    {
        return false;
    }

    result = deckLink->QueryInterface(IID_IDeckLinkConfiguration,
                                      (void **)&deckLinkConfiguration);
    if (result != S_OK)
    {
        std::cerr << "Could not obtain the IDeckLinkConfiguration interface - result = " << result << "\n";
        return false;
    }

    deckLinkInput->SetCallback(this);

    result = deckLinkInput->GetDisplayModeIterator(&displayModeIterator);
    if (result != S_OK)
    {
        std::cerr << "Could not obtain the video output display mode iterator - result = " << result << "\n";
        return false;
    }

    int displayModeCount = 0;

    BMDDisplayMode selectedDisplayMode = bmdModeNTSC;

    while (displayModeIterator->Next(&displayMode) == S_OK)
    {
        if (videoMode == -1 || videoMode == displayModeCount)
        {
            selectedDisplayMode = displayMode->GetDisplayMode();
            break;
        }
        displayModeCount++;
        displayMode->Release();
    }

    BMDPixelFormat pix = bmdFormat8BitYUV;

    result = deckLinkInput->EnableVideoInput(selectedDisplayMode, pix, 0);
    if (result != S_OK)
    {
        std::cerr << "Failed to enable video input\n";
        return false;
    }

    static int audioChannels       = 2;
    static int audioSampleDepth    = 16;

    result = deckLinkInput->EnableAudioInput(bmdAudioSampleRate48kHz,
                                             audioSampleDepth,
                                             audioChannels);
    if (result != S_OK)
    {
        std::cerr << "Failed to enable audio input\n";
        return false;
    }

    result = deckLinkInput->StartStreams();
    if (result != S_OK)
    {
        return false;
    }

    return true;
}

ULONG BMDSplit::AddRef(void)
{
    std::lock_guard<std::mutex> lock(dataMutex);

    refCount++;

    return refCount;
}

ULONG BMDSplit::Release(void)
{
    std::lock_guard<std::mutex> lock(dataMutex);
    refCount--;

    if (refCount == 0)
    {
        delete this;
        return 0;
    }

    return refCount;
}

HRESULT BMDSplit::VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioFrame)
{
    return S_OK;
}

HRESULT BMDSplit::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents events, IDeckLinkDisplayMode *mode,
                                          BMDDetectedVideoInputFormatFlags)
{
    return S_OK;
}
