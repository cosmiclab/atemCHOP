#include "SwitcherMonitor.h"

SwitcherMonitor::SwitcherMonitor(Atem* atem) : atem(atem), mRefCount(1) { }
SwitcherMonitor::~SwitcherMonitor() { }

HRESULT SwitcherMonitor::QueryInterface(REFIID iid, LPVOID* ppv)
{
	if (!ppv)
		return E_POINTER;

	if (IsEqualGUID(iid, IID_IBMDSwitcherCallback))
	{
		*ppv = static_cast<IBMDSwitcherCallback*>(this);
		AddRef();
		return S_OK;
	}

	if (IsEqualGUID(iid, IID_IUnknown))
	{
		*ppv = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}

	*ppv = NULL;
	return E_NOINTERFACE;
}

ULONG SwitcherMonitor::AddRef(void)
{
	return InterlockedIncrement(&mRefCount);
}

ULONG SwitcherMonitor::Release(void)
{
	int newCount = InterlockedDecrement(&mRefCount);
	if (newCount == 0)
		delete this;
	return newCount;
}

// Switcher event callback
HRESULT SwitcherMonitor::Notify(BMDSwitcherEventType eventType, BMDSwitcherVideoMode coreVideoMode)
{
	switch (eventType) {
	case bmdSwitcherEventTypeVideoModeChanged:					OnVideoModeChanged();					break;
	case bmdSwitcherEventTypeMethodForDownConvertedSDChanged:	OnMethodForDownConvertedSDChanged();	break;
	case bmdSwitcherEventTypeDownConvertedHDVideoModeChanged:	OnDownConvertedHDVideoModeChanged();	break;
	case bmdSwitcherEventTypeMultiViewVideoModeChanged:			OnMultiViewVideoModeChanged();			break;
	case bmdSwitcherEventTypePowerStatusChanged:				OnPowerStatusChanged();					break;
	case bmdSwitcherEventTypeDisconnected:						OnDisconnected();						break;
	case bmdSwitcherEventType3GSDIOutputLevelChanged:			On3GSDIOutputLevelChanged();			break;
	case bmdSwitcherEventTypeTimeCodeChanged:					OnTimeCodeChanged();					break;
	case bmdSwitcherEventTypeTimeCodeLockedChanged:				OnTimeCodeLockedChanged();				break;
	case bmdSwitcherEventTypeTimeCodeModeChanged:				OnTimeCodeModeChanged();				break;
	case bmdSwitcherEventTypeSuperSourceCascadeChanged:			OnSuperSourceCascadeChanged();			break;
	case bmdSwitcherEventTypeAutoVideoModeChanged:				OnAutoVideoModeChanged();				break;
	case bmdSwitcherEventTypeAutoVideoModeDetectedChanged:		OnAutoVideoModeDetectedChanged();		break;
	}

	return S_OK;
}

void SwitcherMonitor::OnVideoModeChanged()					{}
void SwitcherMonitor::OnMethodForDownConvertedSDChanged()	{}
void SwitcherMonitor::OnDownConvertedHDVideoModeChanged()	{}
void SwitcherMonitor::OnMultiViewVideoModeChanged()			{}
void SwitcherMonitor::OnPowerStatusChanged()				{}
void SwitcherMonitor::OnDisconnected()						{ atem->OnDisconnect(); }
void SwitcherMonitor::On3GSDIOutputLevelChanged()			{}
void SwitcherMonitor::OnTimeCodeChanged()					{}
void SwitcherMonitor::OnTimeCodeLockedChanged()				{}
void SwitcherMonitor::OnTimeCodeModeChanged()				{}
void SwitcherMonitor::OnSuperSourceCascadeChanged()			{}
void SwitcherMonitor::OnAutoVideoModeChanged()				{}
void SwitcherMonitor::OnAutoVideoModeDetectedChanged()		{}