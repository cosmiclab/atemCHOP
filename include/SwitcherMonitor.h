#pragma once

//#include "../BMDSwitcherAPI.h"
#include "atem.h"

// Callback class to monitor switcher disconnection
class SwitcherMonitor : public IBMDSwitcherCallback
{
public:
	SwitcherMonitor(Atem* atem);

protected:
	virtual ~SwitcherMonitor();

public:
	// IBMDSwitcherCallback interface
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID* ppv);

	ULONG STDMETHODCALLTYPE AddRef(void);

	ULONG STDMETHODCALLTYPE Release(void);

	// Switcher event callback
	HRESULT STDMETHODCALLTYPE	Notify(BMDSwitcherEventType eventType, BMDSwitcherVideoMode coreVideoMode);

private:
	Atem* atem;
	LONG	mRefCount;

	// The video standard changed.
	void OnVideoModeChanged();
	
	// The method for down converted SD output has changed.
	void OnMethodForDownConvertedSDChanged(); 

	// The down converted HD output video standard changed for a particular 
	// core video standard.
	void OnDownConvertedHDVideoModeChanged();
	
	// The MultiView standard changed for a particular core video standard.
	void OnMultiViewVideoModeChanged();
	
	// The power status changed.
	void OnPowerStatusChanged();
	
	// The switcher disconnected.
	void OnDisconnected();
			
	// The 3GSDI output level changed.
	void On3GSDIOutputLevelChanged();
	
	// The current timecode has changed. This only occurs when another event 
	// causes the currently cached timecode to be updated
	void OnTimeCodeChanged();
	
	// The editable status of the timecode has changed.
	void OnTimeCodeLockedChanged();
	
	// The current timecode mode has changed.
	void OnTimeCodeModeChanged();
	
	// The Supersource cascade mode has changed.
	void OnSuperSourceCascadeChanged();
	
	// The auto video mode state has changed.
	void OnAutoVideoModeChanged();
	
	// The auto video mode detection state has changed.
	void OnAutoVideoModeDetectedChanged();

	class SwitchMonitor
	{
	};
};