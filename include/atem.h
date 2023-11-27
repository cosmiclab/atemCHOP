#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <string>
#include <vector>
#include <comutil.h>
#include <atlbase.h>

#pragma comment(lib, "comsuppw.lib")

#include "BMDSwitcherAPI.h"

class SwitcherMonitor;
struct IBMDSwitcherMixEffectBlockCallback;

struct Atem {
    private:
        enum class EConnectionState
        {
            Closed = 0x01,
            Connecting = 0x02,
            Connected = 0x03
        };

        std::string ipAddress;
        Atem::EConnectionState connectionState = Atem::EConnectionState::Closed;
        std::string productName;

        BMDSwitcherConnectToFailure failure;
        IBMDSwitcher* switcher;
        SwitcherMonitor* switcherMonitor;

        std::vector<CComPtr<IBMDSwitcherDownstreamKey>> downstreamKeys;
        std::vector<CComPtr<IBMDSwitcherInput>> inputs;
        std::vector<CComPtr<IBMDSwitcherMixEffectBlock>> mixerEffectBlocks;
                
        std::vector<BOOL> downstreamKeysOnAir;
        int downstreamKeyRates[4] = { 60, 60, 60, 60 };
        std::vector<BMDSwitcherInputId> mixerEffectPreviewIds;
        std::vector<BMDSwitcherInputId> mixerEffectProgramIds;
        std::vector<double> mixerEffectTransitionPositions;

public:
        Atem();
        virtual ~Atem();

        void Connect(std::string ipAddress);
        void OnDisconnect();

    private:
        void LoadDownstreamKeys();
        void LoadInputs();
        void LoadMixerEffects();

    public:
        bool IsClosed()     { return this->connectionState == Atem::EConnectionState::Closed; }
        bool IsConnected()  { return this->connectionState == Atem::EConnectionState::Connected; }
        bool IsConnecting() { return this->connectionState == Atem::EConnectionState::Connecting; }

        size_t GetDownstreamKeyCount() const { return downstreamKeys.size(); }
        size_t GetInputCount() const         { return inputs.size(); }
        std::string GetIpAddress() const     { return ipAddress; }
        std::string GetProductName() const   { return productName; }
        size_t GetMixerEffectCount() const   { return mixerEffectBlocks.size(); }

        inline BMDSwitcherInputId GetMixerEffectProgramId(uint8_t mixerEffectId) const { return mixerEffectProgramIds[mixerEffectId]; }
        inline BMDSwitcherInputId GetMixerEffectPreviewId(uint8_t mixerEffectId) const { return mixerEffectPreviewIds[mixerEffectId]; }
        inline BOOL GetDownstreamKeyOnAir(uint8_t keyerId) const { return downstreamKeysOnAir[keyerId]; }
        inline double GetDownstreamKeyRate(uint8_t keyerId) const { return downstreamKeyRates[keyerId]; }

        inline void SetDownstreamKeyRate(uint8_t keyerId, int rate) { downstreamKeyRates[keyerId] = rate; }

        void ChangeFaderPosition(uint8_t mixerEffectId, double position);
        void PerformAutoTransition(uint8_t mixerEffectId);
        void PerformCut(uint8_t mixerEffectId);
        void PerformDownstreamKeyAutoTransition(uint8_t keyerId);
        void SetPreviewInput(uint8_t mixerEffectId, uint16_t sourceId);
        void SetProgramInput(uint8_t mixerEffectId, uint16_t sourceId);
        void SwitchProgramToPreview(uint8_t mixerEffectId);
        void ToggleDownstreamKey(uint8_t keyerId);
};