#include "Atem.h"

#include "SwitcherMonitor.h"

Atem::Atem() {}

Atem::~Atem() { OnDisconnect(); }

bool Atem::isClosed()     { return this->connectionState == Atem::EConnectionState::Closed; }
bool Atem::isConnected()  { return this->connectionState == Atem::EConnectionState::Connected; }
bool Atem::isConnecting() { return this->connectionState == Atem::EConnectionState::Connecting; }

void Atem::Connect(std::string ipAddress)
{
    if (this->ipAddress == ipAddress && 
        (connectionState == EConnectionState::Connected || connectionState == EConnectionState::Connecting)) { return; }

    this->ipAddress = ipAddress;

    connectionState = EConnectionState::Closed;
    switcherMonitor = new SwitcherMonitor(this);
  
    inputs.clear();
    mixerEffectBlocks.clear();
    downstreamKeys.clear();

    IBMDSwitcherDiscovery* switcherDiscovery = nullptr;
    
    try
    {
        HRESULT result = CoCreateInstance(CLSID_CBMDSwitcherDiscovery, nullptr, CLSCTX_ALL, IID_IBMDSwitcherDiscovery, (void**)&switcherDiscovery);
        
        if (result == S_OK)
        {
            _BMDSwitcherConnectToFailure failure;
            CComBSTR addressString = _com_util::ConvertStringToBSTR(ipAddress.c_str());
            result = switcherDiscovery->ConnectTo(addressString, &switcher, &failure);

            if (result == S_OK && failure <= 0)
            {
                BSTR temp;
                switcher->GetProductName(&temp);
                std::wstring wProductName = temp;
                productName = std::string(wProductName.begin(), wProductName.end());

                std::cout << "Switcher connected" << std::endl;
                switcher->AddCallback(switcherMonitor);
                connectionState = EConnectionState::Connected;
  
                LoadInputs();
                LoadMixerEffects();
                LoadDownstreamKeys();
            }
            else
            {
                std::cout << failure << std::endl;
            }
        }
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
    }
    
    if (switcherDiscovery != nullptr) { switcherDiscovery->Release(); }
    switcher = nullptr;

    //meCallbacks = IBMDSwitcherMixEffectBlockCallback[mixerEffectCount];
}

void Atem::LoadInputs()
{
    inputs.clear();

    if (switcher == nullptr) { return; }

    CComPtr<IBMDSwitcherInputIterator> switcherInputIterator;
    if (switcher->CreateIterator(IID_IBMDSwitcherInputIterator, (void**)&switcherInputIterator) == S_OK)
    {
        CComPtr<IBMDSwitcherInput> switcherInput;
        while (switcherInputIterator->Next(&switcherInput) == S_OK)
        {
            inputs.push_back(std::move(switcherInput));
        }
    }
}

void Atem::LoadMixerEffects()
{
    mixerEffectBlocks.clear();
    mixerEffectProgramIds.clear();
    mixerEffectPreviewIds.clear();
    mixerEffectTransitionPositions.clear();

    if (switcher == nullptr) { return; }

    CComPtr<IBMDSwitcherMixEffectBlockIterator> mixerEffectBlockIterator;
    if (switcher->CreateIterator(IID_IBMDSwitcherMixEffectBlockIterator, (void**)&mixerEffectBlockIterator) == S_OK)
    {
        CComPtr<IBMDSwitcherMixEffectBlock> mixEffectBlock;
        while (mixerEffectBlockIterator->Next(&mixEffectBlock) == S_OK)
        {
            mixerEffectBlocks.push_back(std::move(mixEffectBlock));
        }

        size_t mixerEffectCount = mixerEffectBlocks.size();
        mixerEffectProgramIds.assign(mixerEffectCount, 0);
        mixerEffectPreviewIds.assign(mixerEffectCount, 0);
        mixerEffectTransitionPositions.assign(mixerEffectCount, 0);
    
        for (size_t mixerEffectId = 0; mixerEffectId < mixerEffectCount; ++mixerEffectId)
        {
            mixerEffectBlocks[mixerEffectId]->GetProgramInput(&mixerEffectProgramIds[mixerEffectId]);
            mixerEffectBlocks[mixerEffectId]->GetPreviewInput(&mixerEffectPreviewIds[mixerEffectId]);
            mixerEffectBlocks[mixerEffectId]->GetTransitionPosition(&mixerEffectTransitionPositions[mixerEffectId]);
        }
    }
}

void Atem::LoadDownstreamKeys()
{
    downstreamKeys.clear();
    downstreamKeysOnAir.clear();

    if (switcher == nullptr) { return; }

    CComPtr<IBMDSwitcherDownstreamKeyIterator> downstreamKeyIterator;
    if (switcher->CreateIterator(IID_IBMDSwitcherDownstreamKeyIterator, (void**)&downstreamKeyIterator) == S_OK)
    {
        CComPtr<IBMDSwitcherDownstreamKey> downstreamKey;
        while (downstreamKeyIterator->Next(&downstreamKey) == S_OK)
        {
            downstreamKeys.push_back(std::move(downstreamKey));
        }
    
        size_t downstreamKeyCount = downstreamKeys.size();
        downstreamKeysOnAir.assign(downstreamKeyCount, 0);
    
        for (size_t downstreamKeyId = 0; downstreamKeyId < downstreamKeyCount; ++downstreamKeyId)
        {
            downstreamKeys[downstreamKeyId]->GetOnAir(&downstreamKeysOnAir[downstreamKeyId]);
        }
    }
}

void Atem::OnDisconnect()
{
    connectionState = EConnectionState::Closed;
    std::cout << "DISCONNECTED" << std::endl;

    mixerEffectBlocks.clear();
    mixerEffectProgramIds.clear();
    mixerEffectPreviewIds.clear();
    mixerEffectTransitionPositions.clear();
    
    downstreamKeys.clear();
    downstreamKeysOnAir.clear();
}

void Atem::SwitchProgramToPreview(uint8_t mixerEffectId)
{
    // Get current program and preview into opposite cache
    mixerEffectBlocks[mixerEffectId]->GetProgramInput(&mixerEffectPreviewIds[mixerEffectId]);
    mixerEffectBlocks[mixerEffectId]->GetPreviewInput(&mixerEffectProgramIds[mixerEffectId]);

    // Set new program and preview
    mixerEffectBlocks[mixerEffectId]->SetProgramInput(mixerEffectProgramIds[mixerEffectId]);
    mixerEffectBlocks[mixerEffectId]->SetPreviewInput(mixerEffectPreviewIds[mixerEffectId]);
}

void Atem::PerformCut(uint8_t mixerEffectId)
{
    if (mixerEffectId >= 0 && mixerEffectId < mixerEffectBlocks.size())
    {
        mixerEffectBlocks[mixerEffectId]->PerformCut();

        mixerEffectBlocks[mixerEffectId]->GetProgramInput(&mixerEffectProgramIds[mixerEffectId]);
        mixerEffectBlocks[mixerEffectId]->GetPreviewInput(&mixerEffectPreviewIds[mixerEffectId]);
    }
}

void Atem::PerformAutoTransition(uint8_t mixerEffectId)
{
    if (mixerEffectId >= 0 && mixerEffectId < mixerEffectBlocks.size())
    {
        mixerEffectBlocks[mixerEffectId]->PerformAutoTransition();

        // Switch program and preview cache
        mixerEffectBlocks[mixerEffectId]->GetProgramInput(&mixerEffectPreviewIds[mixerEffectId]);
        mixerEffectBlocks[mixerEffectId]->GetPreviewInput(&mixerEffectProgramIds[mixerEffectId]);
    }
}

void Atem::ChangeFaderPosition(uint8_t mixerEffectId, double position)
{
    if (mixerEffectId >= 0 && mixerEffectId < mixerEffectBlocks.size())
    {
        if (position == mixerEffectTransitionPositions[mixerEffectId]) { return; }
    
        mixerEffectTransitionPositions[mixerEffectId] = position;
        mixerEffectBlocks[mixerEffectId]->SetTransitionPosition(position); 
    }
}

void Atem::SetProgramInput(uint8_t mixerEffectId, uint16_t source)
{
    if (mixerEffectId >= 0 && mixerEffectId < mixerEffectBlocks.size() && source != mixerEffectProgramIds[mixerEffectId])
    {
        mixerEffectBlocks[mixerEffectId]->SetProgramInput(source);
        mixerEffectProgramIds[mixerEffectId] = source;
    }
}

void Atem::SetPreviewInput(uint8_t mixerEffectId, uint16_t source)
{
    if (mixerEffectId >= 0 && mixerEffectId < mixerEffectBlocks.size() && source != mixerEffectPreviewIds[mixerEffectId])
    {
        mixerEffectBlocks[mixerEffectId]->SetPreviewInput(source);
        mixerEffectPreviewIds[mixerEffectId] = source;
    }
}

void Atem::ToggleDownstreamKey(uint8_t downstreamKeyId) // perform a downstream key cut
{
    if (downstreamKeyId >= 0 && downstreamKeyId < downstreamKeys.size())
    {
        BOOL on;
        downstreamKeys[downstreamKeyId]->GetOnAir(&on);
        downstreamKeys[downstreamKeyId]->SetOnAir(!on);
        downstreamKeysOnAir[downstreamKeyId] = !on;
    }
}

void Atem::PerformDownstreamKeyAutoTransition(uint8_t downstreamKeyId) // not quite working yet
{
    if (downstreamKeyId >= 0 && downstreamKeyId < downstreamKeys.size())
    {
        downstreamKeys[downstreamKeyId]->SetRate(downstreamKeyRates[downstreamKeyId]);
        downstreamKeys[downstreamKeyId]->PerformAutoTransition();

        BOOL on;
        downstreamKeys[downstreamKeyId]->GetOnAir(&on);
        downstreamKeysOnAir[downstreamKeyId] = !on;
    }
}