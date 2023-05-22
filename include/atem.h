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

#define connStateClosed 0x01
#define connStateConnecting 0x02
#define connStateConnected 0x03

class SwitcherMonitor;
class IBMDSwitcherMixEffectBlockCallback;

struct Atem {
 public:
  int nofMEs = 0;
  int nofSources = 0;
  int nofCGs = 0;
  int nofAuxs = 0;
  int nofDSKs = 0;
  int nofStingers = 0;
  int nofDVEs = 0;
  int nofSSrcs = 0;
  int nofInputs = 0;
  int nofMacros = 0;
  int lastFaderPos[4];

  std::vector<std::string> chan_names;
  std::vector<float> chan_values;

  std::vector<std::string> topology_names{
      "atemNofMEs",    "atemNofSources",   "atemNofCGs",  "atemNofAuxs",
      "atemNofDSKs",   "atemNofStingers", "atemNofDVEs", "atemNofSSrcs",
      "atemNofInputs", "atemNofMacros"};
  std::vector<float> topology_values{0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

  std::vector<std::string> atem_input_names{};
  std::vector<std::string> atem_input_labels{};

  std::vector<std::string> atem_macro_names{};
  std::vector<int> atem_macro_run_status{};

  uint8_t conn_state;

  std::vector<bool> dcut{false};
  std::vector<bool> daut{false};
  std::vector<uint16_t> cpgi{0};
  std::vector<uint16_t> cpvi{0};
  std::vector<uint16_t> caus{0};
  std::vector<uint16_t> ctps{ 0, 0, 0, 0 }; //***
  std::vector<bool> cdsl{false};
  std::vector<bool> ddsa{false};

  std::vector<CComPtr<IBMDSwitcherMixEffectBlock>> mixEffectBlocks;
  std::vector<CComPtr<IBMDSwitcherDownstreamKey>> downstreamKeys;

  std::string atem_ip = "";
  int active = -1;

  std::string atem_product_id;
  std::string atem_warning;

//  std::ostringstream oss;


  IBMDSwitcher* switcher;
  BMDSwitcherConnectToFailure failure;
  SwitcherMonitor* monitor;
  IBMDSwitcherMixEffectBlockCallback* meCallbacks;


  bool isClosed();
  bool isConnected();
  bool isConnecting();

  void init();
  void initChannels(std::vector<std::string>& outputs);

  void start();
  void stop();

  void performCut(uint8_t me);
  void performAuto(uint8_t me);
  void changeFaderPosition(uint8_t me, double pos, int& dir); //***
  void changeProgramInput(uint8_t me, uint16_t source);
  void changePreviewInput(uint8_t me, uint16_t source);
  void changeAuxSource(uint8_t index, uint16_t source);
  void changeDownstreamKeyer(uint8_t keyer, bool onair);
  void performDownstreamKeyerAuto(uint8_t keyer);

  void updateOutput(uint8_t me);

  void OnDisconnect();

  Atem();
  virtual ~Atem();
};