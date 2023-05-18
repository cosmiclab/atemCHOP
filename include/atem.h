#pragma once

#include <algorithm>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "udp.h"

#include <cstdio>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <conio.h>
#include <comutil.h>
#include <atlbase.h>

#pragma comment(lib, "comsuppw.lib")

#include "BMDSwitcherAPI.h"

#define flagAckRequest 0x01
#define flagHelloPacket 0x02
#define flagResend 0x04
#define flagRequestNextAfter 0x08
#define flagAck 0x10

#define connStateClosed 0x01
#define connStateConnecting 0x02
#define connStateConnected 0x03

#define checkCloseTerm 3
#define checkReconnTerm 3

class SwitcherMonitor;

struct Atem {
 public:
  std::thread send_thread;
  std::thread recv_thread;
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

  std::vector<std::string> chan_names;
  std::vector<float> chan_values;

  std::vector<std::string> topology_names{
      "atemNofMEs",    "atemNofSoures",   "atemNofCGs",  "atemNofAuxs",
      "atemNofDSKs",   "atemNofStingers", "atemNofDVEs", "atemNofSSrcs",
      "atemNofInputs", "atemNofMacros"};
  std::vector<float> topology_values{0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

  std::vector<std::string> atem_input_names{};
  std::vector<std::string> atem_input_labels{};

  std::vector<std::string> atem_macro_names{};
  std::vector<int> atem_macro_run_status{};

  uint8_t conn_state;

  std::vector<uint8_t> start_packet{0x10, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x26, 0x00, 0x00, 0x01, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  std::vector<uint8_t> ack_packet{0x80, 0x0c, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  std::vector<bool> dcut{false};
  std::vector<bool> daut{false};
  std::vector<uint16_t> cpgi{0};
  std::vector<uint16_t> cpvi{0};
  std::vector<uint16_t> caus{0};
  std::vector<uint16_t> ctps{ 0, 0, 0, 0 }; //***
  std::vector<bool> cdsl{false};
  std::vector<bool> ddsa{false};

  std::vector<CComPtr<IBMDSwitcherMixEffectBlock>> mixEffectBlocks;



  uint16_t session_id;
  uint16_t last_session_id;
  uint16_t remote_packet_id;
  uint16_t last_remote_packet_id;
  uint16_t local_packet_id;

  std::queue<std::vector<uint8_t> > que;

  std::string atem_ip = "";
  int atem_port;
  int active = -1;
  time_t last_recv_packet_t;
  time_t last_send_packet_t;

  std::string atem_product_id;
  std::string atem_warning;

  std::ostringstream oss;

  Udp udp;

  IBMDSwitcher* switcher;
  BMDSwitcherConnectToFailure failure;
  SwitcherMonitor* monitor;

  bool isClosed();
  bool isConnected();
  bool isConnecting();

  void init();
  void initChannels(std::vector<std::string>& outputs);

  void start();
  void stop();

  void sendPacket(std::vector<uint8_t> packet);
  void sendPacketStart();
  void sendPacketAck();

  void parsePacket(std::vector<unsigned char> packet);
  void parseCommand(std::vector<uint8_t> packet);
  void readCommand(std::string command, std::vector<uint8_t> data);
  void sendCommand(std::string command, std::vector<uint8_t> data);

  void readCommandTopology(std::vector<uint8_t> data);
  void readCommandProductId(std::vector<uint8_t> data);
  void readCommandWarning(std::vector<uint8_t> data);
  void readCommandInputProperty(std::vector<uint8_t> data);
  void readCommandMacroProperty(std::vector<uint8_t> data);
  void readCommandMacroRunStatus(std::vector<uint8_t> data);

  void readCommandProgramInput(std::vector<uint8_t> data);
  void readCommandPreviewInput(std::vector<uint8_t> data);
  void readCommandAuxSource(std::vector<uint8_t> data);
  void readCommandDownstreamKeyer(std::vector<uint8_t> data);

  void parseSessionID(std::vector<uint8_t> packet);
  void parseRemotePacketID(std::vector<uint8_t> packet);

  uint16_t word(uint8_t n1, uint8_t n2);

  void performCut(uint8_t me);
  void performAuto(uint8_t me);
  void changeFaderPosition(uint8_t me, uint16_t source); //***
  void changeProgramInput(uint8_t me, uint16_t source);
  void changePreviewInput(uint8_t me, uint16_t source);
  void changeAuxSource(uint8_t index, uint16_t source);
  void changeDownstreamKeyer(uint8_t keyer, bool onair);
  void performDownstreamKeyerAuto(uint8_t keyer);

  void OnDisconnect();

  Atem();
  virtual ~Atem();
};
//
//// Callback class to monitor switcher disconnection
//class SwitcherMonitor : public IBMDSwitcherCallback
//{
//public:
//	SwitcherMonitor(Atem* atem) : atem(atem), mRefCount(1) { }
//
//protected:
//	virtual ~SwitcherMonitor() { }
//
//public:
//	// IBMDSwitcherCallback interface
//	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID* ppv)
//	{
//		if (!ppv)
//			return E_POINTER;
//
//		if (IsEqualGUID(iid, IID_IBMDSwitcherCallback))
//		{
//			*ppv = static_cast<IBMDSwitcherCallback*>(this);
//			AddRef();
//			return S_OK;
//		}
//
//		if (IsEqualGUID(iid, IID_IUnknown))
//		{
//			*ppv = static_cast<IUnknown*>(this);
//			AddRef();
//			return S_OK;
//		}
//
//		*ppv = NULL;
//		return E_NOINTERFACE;
//	}
//
//	ULONG STDMETHODCALLTYPE AddRef(void)
//	{
//		return InterlockedIncrement(&mRefCount);
//	}
//
//
//	ULONG STDMETHODCALLTYPE Release(void)
//	{
//		int newCount = InterlockedDecrement(&mRefCount);
//		if (newCount == 0)
//			delete this;
//		return newCount;
//	}
//
//	// Switcher event callback
//	HRESULT STDMETHODCALLTYPE	Notify(BMDSwitcherEventType eventType, BMDSwitcherVideoMode coreVideoMode)
//	{
//		if (eventType == bmdSwitcherEventTypeDisconnected)
//		{
//			atem->OnDisconnect();
//		}
//
//		return S_OK;
//	}
//
//private:
//	Atem* atem;
//	LONG	mRefCount;
//
//	class SwitchMonitor
//	{
//	};
//};