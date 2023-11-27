#include <AtemChop.h>

#include <format>

AtemCHOP::AtemCHOP(const OP_NodeInfo* info)
{
    if (atem == nullptr)
    {
        atem = new Atem();
    }
}

AtemCHOP::~AtemCHOP()
{
    atem = nullptr;
}

void AtemCHOP::threadedConnect(std::string ipAddress)
{
    reconnectTimer = 0;

    if (isThreadFinished)
    {
        futureObj = std::async(std::launch::async, &Atem::Connect, atem, ipAddress);
        isThreadFinished = false;
    }

    if (futureObj.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        isThreadFinished = true;
        futureObj.get();
    }
}

void AtemCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
    ginfo->cookEveryFrameIfAsked = true;
    ginfo->inputMatchIndex = 0;
    ginfo->timeslice = false;
}

bool AtemCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
    if (atem != nullptr && atem->IsConnected())
    {
        setOutputs();

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

void AtemCHOP::getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs, void* reserved1)
{
    if (atem == nullptr || !atem->IsConnected()) return;

    name->setString(outputs[index].c_str());
}

void AtemCHOP::EnableParameters(const OP_Inputs* inputs)
{
    for (int i = 0; i < MAX_MIXER_EFFECT_COUNT; ++i)
    {
        bool enable = atem != nullptr && i < atem->GetMixerEffectCount();
        std::string indexStr = std::to_string(i + 1);

        inputs->enablePar(std::string("Program").append(indexStr).c_str(), enable);
        inputs->enablePar(std::string("Preview").append(indexStr).c_str(), enable);
        inputs->enablePar(std::string("Fader").append(indexStr).c_str(), enable);
        inputs->enablePar(std::string("Cut").append(indexStr).c_str(), enable);
        inputs->enablePar(std::string("Auto").append(indexStr).c_str(), enable);
    }

    for (int i = 0; i < MAX_DOWN_STREAM_KEYER_COUNT; ++i)
    {
        bool enable = atem != nullptr && i < atem->GetDownstreamKeyCount();
        std::string indexStr = std::to_string(i + 1);

        inputs->enablePar(std::string("Dskcut").append(indexStr).c_str(), enable);
        inputs->enablePar(std::string("Dskauto").append(indexStr).c_str(), enable);
        inputs->enablePar(std::string("Dskrate").append(indexStr).c_str(), enable);
    }
}

void AtemCHOP::execute(CHOP_Output* output, const OP_Inputs* inputs, void* reserved)
{
    EnableParameters(inputs);
    
    executeHandleParameters(inputs);
    executeHandleInputs(inputs); 

    if (atem->IsClosed() && reconnectTimer++ > RECONNECT_TIMER)
    {
        std::string ipAddress = inputs->getParString("Atemip");
        threadedConnect(ipAddress);
    }
    else if (atem->IsConnected())
    {
        UpdateOutputs(output);
    }
}

void AtemCHOP::UpdateOutputs(CHOP_Output* output)
{
    int channelIndex = 0;

    for (int mixerEffectId = 0; mixerEffectId < atem->GetMixerEffectCount(); ++mixerEffectId)
    {
        output->channels[channelIndex++][0] = (float)atem->GetMixerEffectProgramId(mixerEffectId);
        output->channels[channelIndex++][0] = (float)atem->GetMixerEffectPreviewId(mixerEffectId);
    }

    for (int downstreamKeyId = 0; downstreamKeyId < atem->GetDownstreamKeyCount(); ++downstreamKeyId)
    {
        output->channels[channelIndex++][0] = (float)atem->GetDownstreamKeyOnAir(downstreamKeyId);
    }
}

bool AtemCHOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
    infoSize->rows = 2;// +atem->nofInputs + atem->nofMacros;
    infoSize->cols = 2;
    infoSize->byColumn = false;
    return true;
}

void AtemCHOP::getInfoDATEntries(int32_t index, int32_t nEntries, OP_InfoDATEntries* entries, void* reserved1)
{
    // Product Id
    if (index == 0)
    {
        entries->values[0]->setString("Product Id");
        entries->values[1]->setString(atem->GetProductName().c_str());
    }
}

void AtemCHOP::appendParameter(OP_NumericParameter& par, OP_ParameterManager* manager, ParameterType type)
{
    OP_ParAppendResult res = OP_ParAppendResult::InvalidSize;
    
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
    
    assert(res == OP_ParAppendResult::Success);
}

void AtemCHOP::addGeneralParameter(std::string page, std::string name, std::string label, OP_ParameterManager* manager, ParameterType type)
{
    if (type == StringParam)
    {
        OP_StringParameter par;
        par.name  = name.c_str();
        par.label = label.c_str();
        par.page  = page.c_str();
        OP_ParAppendResult res = manager->appendString(par);
        assert(res == OP_ParAppendResult::Success);
    }
    else
    {
        OP_NumericParameter par;
        par.name  = name.c_str();
        par.label = label.c_str();
        par.page  = page.c_str();
        appendParameter(par, manager, type);
    }    
}


void AtemCHOP::addMixerParameter(std::string page, std::string name, std::string label, int ind, OP_ParameterManager* manager, ParameterType type, int min, int max)
{
    OP_NumericParameter pa;

    std::string indexStr = std::to_string(ind + 1);
    pa.name  = name.append(indexStr).c_str();
    pa.label = label.append(" ").append(indexStr).c_str();
    pa.page  = page.c_str();
    
    if (min >= 0)
    {
        pa.clampMins[0] = true;
        pa.minSliders[0] = min;
        pa.minValues[0] = min;
    }

    if (max >= 0)
    {
        pa.clampMaxes[0] = true;
        pa.maxSliders[0] = max;
        pa.maxValues[0] = max;
    }

    appendParameter(pa, manager, type);
}

void AtemCHOP::setupParameters(OP_ParameterManager* manager, void* reserved1)
{
    addGeneralParameter("General", "Atemip", "Atem IP", manager, StringParam);
    //addGeneralParameter("General", "Fadermirroring", "Fader Mirroring", manager, ToggleParam);
    
    for (int i = 0; i < MAX_MIXER_EFFECT_COUNT; ++i)
    {
        std::string page = std::string("ME ").append(std::to_string(i + 1));
        addMixerParameter(page, "Program", "Program", i, manager, IntParam); // , 0, int(atem->inputs.size()));
        addMixerParameter(page, "Preview", "Preview", i, manager, IntParam); // , 0, int(atem->inputs.size()));
        addMixerParameter(page, "Fader",   "Fader",   i, manager, FaderParam);
        addMixerParameter(page, "Cut",     "Cut",     i, manager, PulseParam);
        addMixerParameter(page, "Auto",    "Auto",    i, manager, PulseParam);
    }

    for (int i = 0; i < MAX_DOWN_STREAM_KEYER_COUNT; ++i)
    {
        addMixerParameter("DSK", "Dskcut",  "Cut",       i, manager, PulseParam);
        addMixerParameter("DSK", "Dskauto", "Auto",      i, manager, PulseParam);
        addMixerParameter("DSK", "Dskrate", "Auto Rate", i, manager, IntParam);
    }
}

void AtemCHOP::setOutputs()
{
    outputs.clear();

    if (!atem) { return; } // Check if re-init was called

    for (int i = 0; i < atem->GetMixerEffectCount(); ++i)
    {
        std::string indexStr = std::to_string(i + 1);
        outputs.push_back(std::string("prgi").append(indexStr));
        outputs.push_back(std::string("prvi").append(indexStr));
    }

    for (int i = 0; i < atem->GetDownstreamKeyCount(); ++i)
    {
        std::string indexStr = std::to_string(i + 1);
        outputs.push_back(std::string("dsks").append(indexStr));
    }
}

bool AtemCHOP::isNamedParameter(std::string parameterName, std::string correctName, int& index)
{
    bool found = parameterName.rfind(correctName, 0) != std::string::npos;
    index = found ? std::stoi(parameterName.substr(correctName.length())) : -1;
    return found;
}

void AtemCHOP::pulsePressed(const char* name, void* reserved1)
{    
    std::string parameterName = std::string(name);
    int effectIndex = -1;

    if      (isNamedParameter(parameterName, "Cut", effectIndex))     { atem->PerformCut(effectIndex-1); }
    else if (isNamedParameter(parameterName, "Auto", effectIndex))    { atem->PerformAutoTransition(effectIndex-1); }
    else if (isNamedParameter(parameterName, "Dskauto", effectIndex)) { atem->PerformDownstreamKeyAutoTransition(effectIndex-1); }
    else if (isNamedParameter(parameterName, "Dskcut", effectIndex))  { atem->ToggleDownstreamKey(effectIndex-1); }
}

void AtemCHOP::executeHandleParameters(const OP_Inputs* inputs) 
{
    std::string ip = inputs->getParString("Atemip");
    
    if (atem->GetIpAddress() != ip || atem->IsClosed()) 
    {
        threadedConnect(ip);
    }

    for (int i = 0; i < atem->GetMixerEffectCount(); ++i)
    {
        std::string indexStr = std::to_string(i + 1);

        int programId = inputs->getParInt(std::string("Program").append(indexStr).c_str());
        int previewId = inputs->getParInt(std::string("Preview").append(indexStr).c_str());
        
        //if (mixerEffectData[i].bMirrored)
        //{
        //    atem->SetProgramInput(i, previewId);
        //    atem->SetPreviewInput(i, programId);
        //}
        //else
        //{
            atem->SetProgramInput(i, programId);
            atem->SetPreviewInput(i, previewId);
        //}

        double faderPosition = inputs->getParDouble(std::string("Fader").append(indexStr).c_str());
        //bool isMirrored = inputs->getParInt("Fadermirroring");

        if (mixerEffectData[i].lastFaderPosition == faderPosition)
        {
            continue;
        }

        //if (isMirrored)
        //{
        //    if (mixerEffectData[i].bMirrored)
        //    {
        //        faderPosition = 1 - faderPosition;
        //    }

        //    if (faderPosition == 1.0)
        //    {
        //        mixerEffectData[i].bMirrored = !mixerEffectData[i].bMirrored;
        //        faderPosition = 0;
        //        atem->SwitchProgramToPreview(i);
        //    }
        //}

        //faderPosition = meFaderDirections[i] == 1 || !isMirrored ? faderPosition : faderPosition * meFaderDirections[i] + 1.0;
        atem->ChangeFaderPosition(i, faderPosition);
        mixerEffectData[i].lastFaderPosition = faderPosition;
    }

    for (int i = 0; i < MAX_DOWN_STREAM_KEYER_COUNT; ++i)
    {
        int downstreamKeyRate = inputs->getParInt(std::string("Dskrate").append(std::to_string(i + 1)).c_str());
        atem->SetDownstreamKeyRate(i, downstreamKeyRate);
    }
}

void AtemCHOP::executeHandleInputs(const OP_Inputs* inputs) //these remaining functions are to be replaced with params
{ 
    for (int i = 0; i < inputs->getNumInputs(); i++)
    {
        const OP_CHOPInput* cinput = inputs->getInputCHOP(i);
        for (int j = 0; j < cinput->numChannels; j++)
        {
            std::string cname = cinput->getChannelName(j);

            if (!strncmp(cname.c_str(), "caus", 4))
            {
                int index = stoi(cname.substr(4, 1)) - 1;
                uint16_t source = uint16_t(cinput->getChannelData(j)[0]);
            }

            if (!strncmp(cname.c_str(), "ddsa", 4))
            {
                int keyer = stoi(cname.substr(4, 1)) - 1;
                BOOL flag = cinput->getChannelData(j)[0] >= 1;
                if (atem->GetDownstreamKeyCount() > keyer && atem->GetDownstreamKeyOnAir(keyer) != flag)
                {
                    if (flag)
                    {
                        atem->PerformDownstreamKeyAutoTransition(keyer);
                    }
                }
            }
        }
    }
}

extern "C"
{
    DLLEXPORT void FillCHOPPluginInfo(CHOP_PluginInfo* info)
    {
        info->apiVersion = CHOPCPlusPlusAPIVersion;
        info->customOPInfo.opType->setString("Atem");
        info->customOPInfo.opLabel->setString("Atem");
        info->customOPInfo.authorName->setString("Cosmic Lab");
        info->customOPInfo.authorEmail->setString("staff@cosmiclab.jp");

        info->customOPInfo.opIcon->setString("ATM");

        info->customOPInfo.minInputs = 0;
        info->customOPInfo.maxInputs = 1;
    }

    DLLEXPORT CHOP_CPlusPlusBase* CreateCHOPInstance(const OP_NodeInfo* info)
    {
      return new AtemCHOP(info);
    }

    DLLEXPORT void DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
    {
        delete (AtemCHOP*)instance;
    }
};