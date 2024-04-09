#define WIN32_LEAN_AND_MEAN

#include "CHOP_CPlusPlusBase.h"
#include "CPlusPlus_Common.h"
#include "atem.h"
#include <future>
#include <chrono>

class AtemCHOP : public CHOP_CPlusPlusBase
{
private:
	enum ParameterType { IntParam, FloatParam, FaderParam, PulseParam, ToggleParam, StringParam};
	
	Atem* atem = nullptr;
	
	static const int MAX_MIXER_EFFECT_COUNT  = 4; //since we seemingly can't dynamically change parameters based on switcher type...
	static const int MAX_DOWN_STREAM_KEYER_COUNT = 2; //""
	static const int RECONNECT_TIMER = 30; //try to reconnect once every 30 frames
	
	bool isThreadFinished = true;
	int meFaderDirections[4] = { 1, 1, 1, 1 };
	int reconnectTimer = 0;

	std::thread conThr;
	std::future<void> futureObj;
	std::vector<std::string> outputs;
	
	void addGeneralParameter(std::string page, std::string name, std::string label, OP_ParameterManager* manager, ParameterType type);
	void addMixerParameter(std::string page, std::string name, std::string label, int ind, OP_ParameterManager* manager, ParameterType type, int min = -1, int max = -1);
	void appendParameter(OP_NumericParameter& par, OP_ParameterManager* manager, ParameterType type);
	void ConnectToSwitcher(std::string ipAddress);
	void executeHandleInputs(const OP_Inputs* inputs);
	void executeHandleParameters(const OP_Inputs* inputs);
	void setOutputs();
	void threadedConnect(std::string ipAddress);

	bool isNamedParameter(std::string parameterName, std::string correctName, int& index);

 public:
	AtemCHOP(const OP_NodeInfo* info);
	virtual ~AtemCHOP();

	void getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs, void* reserved1);
	void getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1);
	bool getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1);
	void getInfoDATEntries(int32_t index, int32_t nEntries, OP_InfoDATEntries* entries, void* reserved1);
	bool getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1);

	void execute(CHOP_Output* output, const OP_Inputs* inputs, void* reserved);
	void UpdateOutputs(CHOP_Output* output);
	void pulsePressed(const char* name, void* reserved1);
	void setupParameters(OP_ParameterManager* manager, void* reserved1);

private:
	void EnableParameters(const OP_Inputs* inputs);
};