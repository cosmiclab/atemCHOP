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
    public:
        enum class EConnectionState
        {
            Closed = 0x01,
            Connecting = 0x02,
            Connected = 0x03
        };

        //size_t mixerEffectCount = 0;
        //int nofSources = 0;
        //int nofCGs = 0;
        //int nofAuxs = 0;
        //size_t downstreamKeyCount = 0;
        //int nofStingers = 0;
        //int nofDVEs = 0;
        //int nofSSrcs = 0;
        //int nofInputs = 0;
        //int nofMacros = 0;

        std::vector<double> mixerEffectTransitionPositionsOld;
        int downstreamKeyRates[4] = { 60, 60, 60, 60 };

        std::vector<std::string> atem_input_names{};
        std::vector<std::string> atem_input_labels{};

        std::vector<std::string> atem_macro_names{};
        std::vector<int> atem_macro_run_status{};

        Atem::EConnectionState connectionState = Atem::EConnectionState::Closed;
        
        std::vector<BMDSwitcherInputId> mixerEffectProgramIds;
        std::vector<BMDSwitcherInputId> mixerEffectPreviewIds;
        std::vector<double> mixerEffectTransitionPositions;
        std::vector<BOOL> downstreamKeysOnAir;

        std::vector<CComPtr<IBMDSwitcherMixEffectBlock>> mixerEffectBlocks;
        std::vector<CComPtr<IBMDSwitcherDownstreamKey>> downstreamKeys;
        std::vector<CComPtr<IBMDSwitcherInput>> inputs;

        
        int active = -1;

        //std::ostringstream oss;

        IBMDSwitcher* switcher;
        BMDSwitcherConnectToFailure failure;
        SwitcherMonitor* switcherMonitor;
        //IBMDSwitcherMixEffectBlockCallback* meCallbacks;

        bool isClosed();
        bool isConnected();
        bool isConnecting();

        void Connect(std::string ipAddress);

        void start();
        void stop();

        void PerformCut(uint8_t me);
        void PerformAutoTransition(uint8_t me);
        void ChangeFaderPosition(uint8_t me, double pos); //***
        void SetProgramInput(uint8_t me, uint16_t source);
        void SetPreviewInput(uint8_t me, uint16_t source);

        void ToggleDownstreamKey(uint8_t keyer/*, bool onair*/);
        void PerformDownstreamKeyAutoTransition(uint8_t keyer);

        void SwitchProgramToPreview(uint8_t mixerEffectId);

        void OnDisconnect();

        Atem();
        virtual ~Atem();

        size_t GetDownstreamKeyCount() { return downstreamKeys.size(); }
        size_t GetInputCount() { return inputs.size(); }
        size_t GetMixerEffectCount() { return mixerEffectBlocks.size(); }

        std::string GetIpAddress() { return ipAddress; }
        std::string GetProductName() { return productName; }

    private:
        std::string ipAddress;

        std::string productName;

        void LoadDownstreamKeys();
        void LoadInputs();
        void LoadMixerEffects();
};