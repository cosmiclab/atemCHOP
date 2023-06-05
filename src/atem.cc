#include "atem.h"

#include "SwitcherMonitor.h"

Atem::Atem() {}

Atem::~Atem() {}

bool Atem::isClosed() { return this->conn_state == connStateClosed; }
bool Atem::isConnected() { return this->conn_state == connStateConnected; }
bool Atem::isConnecting() { return this->conn_state == connStateConnecting; }

void Atem::initChannels(std::vector<std::string>& outputs) //initialize output channels with correct values
{
    chan_values.clear();
    chan_names.clear();
    chan_values.assign(outputs.size(), 0.0f);
    for (size_t i = 0; i < outputs.size(); ++i)
    {
        chan_names.push_back(outputs[i]);
    }
    
    nofMEs = mixEffectBlocks.size();
    cpgi.assign(nofMEs, 0);
    cpvi.assign(nofMEs, 0);
    dcut.assign(nofMEs, false);
    daut.assign(nofMEs, false);
    ctps.assign(nofMEs, 0);

    nofDSKs = downstreamKeys.size();
    cdsl.assign(nofDSKs, 0);
    ddsa.assign(nofDSKs, 0);

    //meCallbacks = IBMDSwitcherMixEffectBlockCallback[nofMEs];
    
    for (size_t i = 0; i < nofMEs; ++i)
    {
        BMDSwitcherInputId pgm = -1;
        BMDSwitcherInputId pvw = -1;
        double pos = -1.0;
        mixEffectBlocks[i]->GetProgramInput(&pgm);
        mixEffectBlocks[i]->GetPreviewInput(&pvw);
        mixEffectBlocks[i]->GetTransitionPosition(&pos);
        cpgi[i] = pgm;
        cpvi[i] = pvw;
        ctps[i] = pos * 1000;
        chan_values[i * 2] = pgm;
        chan_values[i * 2 + 1] = pvw;
    }
    for (size_t i = 0; i < nofDSKs; ++i)
    {
        BOOL on;
        downstreamKeys[i]->GetOnAir(&on);
        cdsl[i] = on;
    }
}

bool Atem::isMixerAmountChanged(int amount)
{
    if (amount != chan_values.size())
    {
        return true;
    }
    return false;
}

void Atem::init() {

  conn_state = connStateClosed;

  topology_values.assign(topology_values.size(), 0.0f);

  monitor = new SwitcherMonitor(this);
  

  //udp.setup(atem_ip);
  try {
      IBMDSwitcherDiscovery* switcherDiscovery = NULL;
      HRESULT result = CoCreateInstance(CLSID_CBMDSwitcherDiscovery, NULL, CLSCTX_ALL, IID_IBMDSwitcherDiscovery, (void**)&switcherDiscovery);
      if (result == S_OK)
      {
          CComBSTR addressString = _com_util::ConvertStringToBSTR(atem_ip.c_str());
          failure = _BMDSwitcherConnectToFailure();
          result = switcherDiscovery->ConnectTo(addressString, &switcher, &failure);
          if (result == S_OK && failure <= 0)
          {
              std::cout << "Switcher connected" << std::endl;
              switcher->AddCallback(monitor);
              conn_state = connStateConnected;
  
              mixEffectBlocks.clear();
              downstreamKeys.clear();

              CComPtr<IBMDSwitcherMixEffectBlockIterator> mixEffectBlockIterator;
              if (switcher->CreateIterator(IID_IBMDSwitcherMixEffectBlockIterator, (void**)&mixEffectBlockIterator) == S_OK)
              {
                  CComPtr<IBMDSwitcherMixEffectBlock> mixEffectBlock;
                  while (mixEffectBlockIterator->Next(&mixEffectBlock) == S_OK)
                      mixEffectBlocks.push_back(std::move(mixEffectBlock));
                        
              }

              CComPtr<IBMDSwitcherDownstreamKeyIterator> downstreamKeyIterator;
              if (switcher->CreateIterator(IID_IBMDSwitcherDownstreamKeyIterator, (void**)&downstreamKeyIterator) == S_OK)
              {
                  CComPtr<IBMDSwitcherDownstreamKey> downstreamKey;
                  while (downstreamKeyIterator->Next(&downstreamKey) == S_OK)
                      downstreamKeys.push_back(std::move(downstreamKey));
              }
          }
          else
          {
              mixEffectBlocks.clear();
              downstreamKeys.clear();
              switcher = nullptr;
              std::cout << failure << std::endl;
          }

      }

      if (switcherDiscovery != NULL) { switcherDiscovery->Release(); }
  }
  catch (std::exception e)
  {
      std::cout << e.what() << std::endl;
  }
}

void Atem::OnDisconnect()
{
    conn_state = connStateClosed;
    std::cout << "DISCONNECTED" << std::endl;

    chan_values.clear();
    chan_names.clear();
    nofMEs = 0;
    cpgi.assign(nofMEs, 0);
    cpvi.assign(nofMEs, 0);
}

void Atem::start() {
  std::cout << "thread start" << std::endl;
  active = true;
}

void Atem::stop() {
    std::cout << "thread stop" << std::endl;
  active = false;

  conn_state = connStateClosed;
}

void Atem::updateOutput(uint8_t me)
{
    BMDSwitcherInputId pgm = -1;
    BMDSwitcherInputId pvw = -1;
    mixEffectBlocks[me]->GetProgramInput(&pgm);
    mixEffectBlocks[me]->GetPreviewInput(&pvw);

    mixEffectBlocks[me]->SetProgramInput(pvw); //swap
    mixEffectBlocks[me]->SetPreviewInput(pgm);

    chan_values[me * 2] = pvw; //swap
    chan_values[me * 2 + 1] = pgm;
}

void Atem::performCut(uint8_t me) {
  
  if (me >= 0 && me < nofMEs)
  {
      mixEffectBlocks[me]->PerformCut();
      updateOutput(me);
  }
}

void Atem::performAuto(uint8_t me) {

  if (me >= 0 && me < nofMEs)
  {
      mixEffectBlocks[me]->PerformAutoTransition();

      BMDSwitcherInputId pgm = -1;
      BMDSwitcherInputId pvw = -1;
      mixEffectBlocks[me]->GetProgramInput(&pgm);
      mixEffectBlocks[me]->GetPreviewInput(&pvw);
      chan_values[me * 2] = pvw; //swap
      chan_values[me * 2 + 1] = pgm;
      //updateOutput(me);
  }
}

void Atem::changeFaderPosition(uint8_t me, double pos, int& dir) //changed pos to double 
{
    //double curPos;
    //mixEffectBlocks[me]->GetTransitionPosition(&curPos);
    if (pos == lastFaderPos[me])// && pos == curPos)
    {
    }
    else if (me >= 0 && me < nofMEs)
    {
        ctps[me] = pos;
        mixEffectBlocks[me]->SetTransitionPosition(pos); 
        if (pos == 1.0)
        {
            updateOutput(me);
            dir *= -1;
        }
    }
    lastFaderPos[me] = pos;
    return;
}

void Atem::changeProgramInput(uint8_t me, uint16_t source) {
    
  if (me >= 0 && me < cpgi.size() && me < nofMEs && source != cpgi[me])
  {
      cpgi[me] = source;
      mixEffectBlocks[me]->SetProgramInput(source);
      chan_values[me*2] = source;
  }
}

void Atem::changePreviewInput(uint8_t me, uint16_t source) {
    if (me >= 0 && me < cpvi.size() && me < nofMEs && source != cpvi[me])
    {
        cpvi[me] = source;
        mixEffectBlocks[me]->SetPreviewInput(source);
        chan_values[me * 2+1] = source;
    }
}

void Atem::changeAuxSource(uint8_t index, uint16_t source) {
  //caus[index] = source;

  //std::vector<uint8_t> data{1, index};
  //data.push_back(source >> 0x08);
  //data.push_back(source & 0xff);

  //sendCommand("CAuS", data);
}

void Atem::changeDownstreamKeyer(uint8_t keyer) { //perform a dsk cut

    if (keyer >= 0 && keyer < nofDSKs)
    {
        BOOL on;
        downstreamKeys[keyer]->GetOnAir(&on);
        downstreamKeys[keyer]->SetOnAir(!on);
        cdsl[keyer] = !on;
        chan_values[nofMEs * 2 + keyer] = !on ? 1 : 0;
    }
}

void Atem::performDownstreamKeyerAuto(uint8_t keyer) { //not quite working yet

    if (keyer >= 0 && keyer < nofDSKs)
    {
        downstreamKeys[keyer]->SetRate(dskRates[keyer]);
        downstreamKeys[keyer]->PerformAutoTransition();

        BOOL on;
        downstreamKeys[keyer]->GetOnAir(&on);
        cdsl[keyer] = !on;
        chan_values[nofMEs * 2 + keyer] = !on ? 1 : 0;
        //chan_values[me * 2] = pvw; //swap
        //chan_values[me * 2 + 1] = pgm;
    }
}
