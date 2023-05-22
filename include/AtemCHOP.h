#define WIN32_LEAN_AND_MEAN

#include "CHOP_CPlusPlusBase.h"
#include "CPlusPlus_Common.h"
#include "atem.h"
#include <future>
#include <chrono>

class AtemCHOP : public CHOP_CPlusPlusBase {
 private:
  Atem* atem;

  enum ParameterType { IntParam, FloatParam, FaderParam, PulseParam, ToggleParam, StringParam};

  void executeHandleParameters(const OP_Inputs* inputs);
  void executeHandleInputs(const OP_Inputs* inputs);
  void addMixerParameter(std::string page, std::string name, std::string label, int ind, OP_ParameterManager* manager, ParameterType type);
  void addGeneralParameter(std::string page, std::string name, std::string label, OP_ParameterManager* manager, ParameterType type);

  void appendParameter(OP_NumericParameter& par, OP_ParameterManager* manager, ParameterType type);

  void setOutputs();

  std::vector<std::string> outputs;
  std::vector<std::string> oneOutput = { "prgi", "prvi" };
  int maxMEs = 4; //since we seemingly can't dynamically change parameters based on switcher type...
  int maxDSKs = 2; //""

  int meFaderDirections[4] = { 1 };

  int reconnectTimer = 0;
  int connectTimerDur = 30; //try to reconnect once every 30 frames
  std::future<void> futureObj;
  std::thread conThr;
  bool isThreadFinished = true;
  void connect();
  void threadedConnect();

 public:
  AtemCHOP(const OP_NodeInfo* info);
  virtual ~AtemCHOP();

  void getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs,
                      void* reserved1);
  bool getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs,
                     void* reserved1);
  void getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs,
                      void* reserved1);
  void execute(CHOP_Output* output, const OP_Inputs* inputs, void* reserved);
  int32_t getNumInfoCHOPChans(void* reserved1);
  void getInfoCHOPChan(int32_t index, OP_InfoCHOPChan* chan, void* reserved1);
  bool getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1);
  void getInfoDATEntries(int32_t index, int32_t nEntries,
                         OP_InfoDATEntries* entries, void* reserved1);
  void setupParameters(OP_ParameterManager* manager, void* reserved1);
  void pulsePressed(const char* name, void* reserved1);
};
