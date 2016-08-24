//
//  BMD split
//

#include <iostream>
#include "BMDSplit.h"
#include "Utils.h"

BMDSplit::BMDSplit(cppsocket::Network& pNetwork, uint16_t pPort):
    network(pNetwork), port(pPort), socket(pNetwork)
{
}

BMDSplit::~BMDSplit()
{
    if (deckLinkConfiguration) deckLinkConfiguration->Release();
    if (displayMode) displayMode->Release();
    if (displayModeIterator) displayModeIterator->Release();
    if (deckLinkInput) deckLinkInput->Release();
    if (deckLink) deckLink->Release();
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

    result = deckLink->QueryInterface(IID_IDeckLinkInput,
                                      reinterpret_cast<void**>(&deckLinkInput));
    if (result != S_OK)
    {
        return false;
    }

    result = deckLink->QueryInterface(IID_IDeckLinkConfiguration,
                                      reinterpret_cast<void**>(&deckLinkConfiguration));
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

    while (displayModeIterator->Next(&displayMode) == S_OK)
    {
        if (videoMode == -1 || videoMode == displayModeCount)
        {
            selectedDisplayMode = displayMode->GetDisplayMode();
            break;
        }
        displayModeCount++;
        displayMode->Release();
        displayMode = nullptr;
    }

    if (!displayMode)
    {
        std::cerr << "Failed to find display mode\n";
        return false;
    }

    width = displayMode->GetWidth();
    height = displayMode->GetHeight();
    displayMode->GetFrameRate(&frameDuration, &timeScale);
    fieldDominance = displayMode->GetFieldDominance();

    result = deckLinkInput->EnableVideoInput(selectedDisplayMode, pixelFormat, 0);
    if (result != S_OK)
    {
        std::cerr << "Failed to enable video input\n";
        return false;
    }

    result = deckLinkInput->EnableAudioInput(audioSampleRate,
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

    socket.setAcceptCallback(std::bind(&BMDSplit::acceptCallback, this, std::placeholders::_1));
    socket.startAccept(port);

    return true;
}

ULONG BMDSplit::AddRef()
{
    std::lock_guard<std::mutex> lock(dataMutex);

    refCount++;

    return refCount;
}

ULONG BMDSplit::Release()
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

HRESULT BMDSplit::VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame,
                                         IDeckLinkAudioInputPacket* audioFrame)
{
    BMDTimeValue timestamp;

    if (videoFrame)
    {
        if (videoFrame->GetFlags() & static_cast<BMDFrameFlags>(bmdFrameHasNoInputSource))
        {
            return S_OK;
        }
        else
        {
            BMDTimeValue duration;

            uint8_t* frameData;
            videoFrame->GetBytes(reinterpret_cast<void**>(&frameData));
            videoFrame->GetStreamTime(&timestamp, &duration, timeScale);

            //uint32_t frameWidth = static_cast<uint32_t>(videoFrame->GetWidth());
            uint32_t frameHeight = static_cast<uint32_t>(videoFrame->GetHeight());
            uint32_t stride = static_cast<uint32_t>(videoFrame->GetRowBytes());

            std::vector<uint8_t> data;
            encodeInt(data, sizeof(timestamp), timestamp);
            data.insert(data.end(), frameData, frameData + frameHeight * stride);

            // send it to all clients
            for (cppsocket::Socket& client : clients)
            {
                client.send(data);
            }
        }
    }

    if (audioFrame)
    {
        uint8_t* frameData;

        long sampleFrameCount = audioFrame->GetSampleFrameCount();
        audioFrame->GetBytes(reinterpret_cast<void**>(&frameData));
        audioFrame->GetPacketTime(&timestamp, audioSampleRate);

        std::vector<uint8_t> data;
        encodeInt(data, sizeof(timestamp), timestamp);
        data.insert(data.end(), frameData, frameData + sampleFrameCount * audioChannels * (audioSampleDepth / 8));

        // send it to all clients
        for (cppsocket::Socket& client : clients)
        {
            client.send(data);
        }
    }

    return S_OK;
}

HRESULT BMDSplit::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*,
                                          BMDDetectedVideoInputFormatFlags)
{
    return S_OK;
}

void BMDSplit::acceptCallback(cppsocket::Socket& client)
{
    client.setCloseCallback([&client] {
        std::cout << "Client disconnected\n";
    });

    std::vector<uint8_t> data;
    encodeInt(data, sizeof(uint32_t), static_cast<uint32_t>(width));
    encodeInt(data, sizeof(uint32_t), static_cast<uint32_t>(height));
    encodeInt(data, sizeof(frameDuration), frameDuration); // numerator
    encodeInt(data, sizeof(timeScale), timeScale); // denumerator
    encodeInt(data, sizeof(fieldDominance), fieldDominance);
    encodeInt(data, sizeof(audioSampleRate), audioSampleRate);
    encodeInt(data, sizeof(audioSampleDepth), audioSampleDepth);
    encodeInt(data, sizeof(audioChannels), audioChannels);
    client.send(data);

    clients.push_back(std::move(client));
}
