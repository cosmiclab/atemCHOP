#include <AtemChop.h>


AtemCHOP::AtemCHOP(const OP_NodeInfo* info) {
  atem = new Atem();
  connect();
}

AtemCHOP::~AtemCHOP() { atem->stop(); }

void AtemCHOP::connect()
{
    atem->init();
    setOutputs();
    atem->initChannels(outputs);
    //std::this_thread::sleep_for(std::chrono::seconds(2));
}

void AtemCHOP::threadedConnect()
{
    if (isThreadFinished)
    {
        futureObj = std::async(std::launch::async, &AtemCHOP::connect, this);
        isThreadFinished = false;
    }

    if (futureObj.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        isThreadFinished = true;
        futureObj.get();
    }
}

void AtemCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs,
                              void* reserved1) {
  ginfo->cookEveryFrameIfAsked = true;
  ginfo->timeslice = false;
  ginfo->inputMatchIndex = 0;
}

bool AtemCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs,
                             void* reserved1) {
    if (atem->isConnected())
    {
        info->numSamples = 1;
        info->numChannels = int32_t(outputs.size());
    }
    else
    {
        info->numChannels = 0;
        info->numSamples = 0;
    }
  
  return true;
}

void AtemCHOP::getChannelName(int32_t index, OP_String* name,
                              const OP_Inputs* inputs, void* reserved1) {
    if (!atem->isConnected()) return;
    name->setString(atem->chan_names[index].c_str());
}

void AtemCHOP::enableParameters(const OP_Inputs* inputs)
{
    for (int i = 0; i < maxMEs; ++i)
    {
        bool enable = i < atem->nofMEs;
        std::string par = "Program" + std::to_string(i + 1);
        inputs->enablePar(par.c_str(), enable);
        par = "Preview" + std::to_string(i + 1);
        inputs->enablePar(par.c_str(), enable);
        par = "Fader" + std::to_string(i + 1);
        inputs->enablePar(par.c_str(), enable);
        par = "Cut" + std::to_string(i + 1);
        inputs->enablePar(par.c_str(), enable);
        par = "Auto" + std::to_string(i + 1);
        inputs->enablePar(par.c_str(), enable);
    }

    for (int i = 0; i < maxDSKs; ++i)
    {
        bool enable = i < atem->nofDSKs;
        std::string par = "Dskcut" + std::to_string(i + 1);
        inputs->enablePar(par.c_str(), enable);
        par = "Dskauto" + std::to_string(i + 1);
        inputs->enablePar(par.c_str(), enable);
        par = "Dskrate" + std::to_string(i + 1);
        inputs->enablePar(par.c_str(), enable);
    }

}

void AtemCHOP::execute(CHOP_Output* output, const OP_Inputs* inputs,
                       void* reserved) { //this is called every frame
    
   // if (atem->isMixerAmountChanged(outputs.size()))
    //{
        enableParameters(inputs);
        setOutputs();
    //}
    executeHandleParameters(inputs);
    executeHandleInputs(inputs); 

    time_t now_t = time(NULL);

    if (atem->isClosed())
    {
        if (reconnectTimer++ > connectTimerDur /*&& !conThr.joinable()*/)
        {          
            threadedConnect();
            reconnectTimer = 0;
        }
    }

    else if (atem->isConnected())
    {
        for (int i = 0; i < atem->chan_values.size(); i++) 
        {          
            for (int j = 0; j < output->numSamples; j++) {      
                output->channels[i][j] = atem->chan_values[i];
            }
        }
    }
}

int32_t AtemCHOP::getNumInfoCHOPChans(void* reserved1) {
  int chans = 3 + (int)atem->topology_values.size();
  return chans;
}

void AtemCHOP::getInfoCHOPChan(int32_t index, OP_InfoCHOPChan* chan,
                               void* reserved1) {
  /*if (index == 0) {
    chan->name->setString("atemSessionID");
    chan->value = (float)atem->session_id;
  }
  if (index == 1) {
    chan->name->setString("atemRemotePacketID");
    chan->value = (float)atem->remote_packet_id;
  }
  if (index == 2) {
    chan->name->setString("atemLocalPacketID");
    chan->value = (float)atem->local_packet_id;
  }
  if (index >= 3) {
    chan->name->setString(
        atem->topology_names[int(static_cast<__int64>(index) - 3)].c_str());
    chan->value =
        (float)atem->topology_values[int(static_cast<__int64>(index) - 3)];
  }*/
}

bool AtemCHOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1) {
  infoSize->rows = 2 + atem->nofInputs + atem->nofMacros;
  infoSize->cols = 3;
  infoSize->byColumn = false;
  return true;
}

void AtemCHOP::getInfoDATEntries(int32_t index, int32_t nEntries,
                                 OP_InfoDATEntries* entries, void* reserved1) {
  // Product Id
  if (index == 0) {
    entries->values[0]->setString("Product Id");
    entries->values[1]->setString(atem->atem_product_id.c_str());
    entries->values[2]->setString("");
  }

  // Warning
  else if (index == 1) {
    entries->values[0]->setString("Warning");
    entries->values[1]->setString(atem->atem_warning.c_str());
    entries->values[2]->setString("");
  }

  // Input Property
  else if (index < 2 + atem->nofInputs) {
    int i = index - 2;
    char key[10];
    sprintf_s(key, "Input%d", int(i + 1));
    entries->values[0]->setString(key);
    entries->values[1]->setString(atem->atem_input_names[i].c_str());
    entries->values[2]->setString(atem->atem_input_labels[i].c_str());
  }

  // Macro Property
  else if (index < 2 + atem->nofInputs + atem->nofMacros) {
    int i = index - 2 - atem->nofInputs;
    char key[10];
    sprintf_s(key, "Macro%d", int(i + 1));
    std::string s = atem->atem_macro_run_status[i] > 0 ? "running" : "";

    entries->values[0]->setString(key);
    entries->values[1]->setString(atem->atem_macro_names[i].c_str());
    entries->values[2]->setString(s.c_str());
  }
}

void AtemCHOP::appendParameter(OP_NumericParameter& par, OP_ParameterManager* manager, ParameterType type)
{
    OP_ParAppendResult res;
    switch (type)
    {
    case IntParam:
        res = manager->appendInt(par);
        break;
    case FloatParam:
        res = manager->appendFloat(par);
        break;
    case PulseParam:
        res = manager->appendPulse(par);
        break;
    case FaderParam:
        par.minSliders[0] = 0.0;
        par.maxSliders[0] = 1.0;// 000;
        par.clampMins[0] = true;
        par.clampMaxes[0] = true;
        par.minValues[0] = 0.0;
        par.maxValues[0] = 1.0;// 000;
        res = manager->appendFloat(par);
        break;
    case ToggleParam:
        res = manager->appendToggle(par);
        break;
    }
    par.name;
    assert(res == OP_ParAppendResult::Success);
}

void AtemCHOP::addGeneralParameter(std::string page, std::string name, std::string label, OP_ParameterManager* manager, ParameterType type)
{
    std::string n = name;
    std::string l = label;
    std::string p = page;

    if (type == StringParam)
    {
        OP_StringParameter par;
        par.name = n.c_str();
        par.label = l.c_str();
        par.page = p.c_str();
        OP_ParAppendResult res = manager->appendString(par);
        assert(res == OP_ParAppendResult::Success);
    }
    else
    {
        OP_NumericParameter par;
        par.name = n.c_str();
        par.label = l.c_str();
        par.page = p.c_str();
        appendParameter(par, manager, type);
    }    
}


void AtemCHOP::addMixerParameter(std::string page, std::string name, std::string label, int ind, OP_ParameterManager* manager, ParameterType type)
{
    OP_NumericParameter pa;
    std::string n = name + std::to_string(ind + 1);
    pa.name = n.c_str();
    std::string l = label + " " + std::to_string(ind + 1);
    pa.label = l.c_str();
    std::string p = page;
    pa.page = p.c_str();
    appendParameter(pa, manager, type);
   
}

void AtemCHOP::setupParameters(OP_ParameterManager* manager, void* reserved1) {
  {
    addGeneralParameter("General", "Atemip", "Atem IP", manager, StringParam);
    addGeneralParameter("General", "Fadermirroring", "Fader Mirroring", manager, ToggleParam);
    
    for (int i = 0; i < maxMEs; ++i) //assuming max four MEs for now...
    {
        std::string page = "ME " + std::to_string(i + 1);
        addMixerParameter(page, "Program", "Program", i, manager, IntParam);
        addMixerParameter(page, "Preview", "Preview", i, manager, IntParam);
        addMixerParameter(page, "Fader", "Fader", i, manager, FaderParam);
        addMixerParameter(page, "Cut", "Cut", i, manager, PulseParam);
        addMixerParameter(page, "Auto", "Auto", i, manager, PulseParam);
    }

    for (int i = 0; i < maxDSKs; ++i)
    {
        addMixerParameter("DSK", "Dskcut", "Cut", i, manager, PulseParam);
        addMixerParameter("DSK", "Dskauto", "Auto", i, manager, PulseParam);
        addMixerParameter("DSK", "Dskrate", "Auto Rate", i, manager, IntParam);
    }
  }
}

void AtemCHOP::setOutputs()
{
    outputs.clear();
    for (int i = 0; i < atem->mixEffectBlocks.size(); ++i)
    {
        for (int j = 0; j < oneMEOutput.size(); ++j)
        {
            outputs.push_back(oneMEOutput[j]+ std::to_string(i+1));
        }
    }
    for (int i = 0; i < maxDSKs; ++i)
    {
        for (int j = 0; j < oneDSKOutput.size(); ++j)
        {
            outputs.push_back(oneDSKOutput[j] + std::to_string(i + 1));
        }
    }
}

void AtemCHOP::pulsePressed(const char* name, void* reserved1)
{    
    if (std::string(name).find("Cut") != std::string::npos)
    {
        int me = std::stoi(std::string(name).substr(3, 1)) - 1;
        atem->performCut(me);
    }

    if (std::string(name).find("Auto") != std::string::npos)
    {
        int me = std::stoi(std::string(name).substr(4, 1)) - 1;
        atem->performAuto(me);
    }

    if (std::string(name).find("Dskauto") != std::string::npos)
    {
        int k = std::stoi(std::string(name).substr(7, 1)) - 1;
        atem->performDownstreamKeyerAuto(k);
    }

    if (std::string(name).find("Dskcut") != std::string::npos)
    {
        int k = std::stoi(std::string(name).substr(6, 1)) - 1;
        atem->changeDownstreamKeyer(k);
    }

}

void AtemCHOP::executeHandleParameters(const OP_Inputs* inputs) 
{
  std::string ip = inputs->getParString("Atemip");
  if (atem->atem_ip != ip || atem->active == -1) 
  {
    atem->atem_ip = ip;
    if (atem->active) atem->stop();
    atem->start();

    threadedConnect();
  }

  for (int i = 0; i < atem->nofMEs; ++i)
  {
      int pg = inputs->getParInt(("Program" + std::to_string(i+1)).c_str());
      atem->changeProgramInput(i, pg);
      
      int pv = inputs->getParInt(("Preview" + std::to_string(i + 1)).c_str());
      atem->changePreviewInput(i, pv);
      
      double pf = inputs->getParDouble(("Fader" + std::to_string(i + 1)).c_str());
      bool mir = inputs->getParInt("Fadermirroring");
      pf = meFaderDirections[i] == 1 || !mir ? pf : pf * meFaderDirections[i] + 1.0;
      atem->changeFaderPosition(i, pf, meFaderDirections[i]);
  }  

  for (int i = 0; i < maxDSKs; ++i)
  {
      int rt = inputs->getParInt(("Dskrate" + std::to_string(i + 1)).c_str());
      atem->dskRates[i] = rt;
  }
}

void AtemCHOP::executeHandleInputs(const OP_Inputs* inputs) { //these remaining functions are to be replaced with params
  for (int i = 0; i < inputs->getNumInputs(); i++) {
    const OP_CHOPInput* cinput = inputs->getInputCHOP(i);
    for (int j = 0; j < cinput->numChannels; j++) {
      std::string cname = cinput->getChannelName(j);

      if (!strncmp(cname.c_str(), "caus", 4)) {
        int index = stoi(cname.substr(4, 1)) - 1;
        uint16_t source = uint16_t(cinput->getChannelData(j)[0]);
        if (atem->nofAuxs > index && atem->caus[index] != source) {
          atem->changeAuxSource(index, source);
        }
      }

      if (!strncmp(cname.c_str(), "ddsa", 4)) {
        int keyer = stoi(cname.substr(4, 1)) - 1;
        bool flag = cinput->getChannelData(j)[0] >= 1;
        if (atem->nofDSKs > keyer && atem->ddsa[keyer] != flag) {
          atem->ddsa[keyer] = flag;
          if (flag) atem->performDownstreamKeyerAuto(keyer);
        }
      }
    }
  }
}

extern "C" {
DLLEXPORT void FillCHOPPluginInfo(CHOP_PluginInfo* info) {
  info->apiVersion = CHOPCPlusPlusAPIVersion;
  info->customOPInfo.opType->setString("Atem");
  info->customOPInfo.opLabel->setString("Atem");
  info->customOPInfo.authorName->setString("Cosmic Lab");
  info->customOPInfo.authorEmail->setString("stephan@cosmiclab.jp");

  info->customOPInfo.opIcon->setString("ATM");

  info->customOPInfo.minInputs = 0;
  info->customOPInfo.maxInputs = 1;
}

DLLEXPORT CHOP_CPlusPlusBase* CreateCHOPInstance(const OP_NodeInfo* info) {
  return new AtemCHOP(info);
}

DLLEXPORT void DestroyCHOPInstance(CHOP_CPlusPlusBase* instance) {
  delete (AtemCHOP*)instance;
}
};
