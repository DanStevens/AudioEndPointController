#pragma once
#ifndef _AUDIODEVICECONTROLLER_
#define _AUDIODEVICECONTROLLER_

#include <map>
#include "windows.h"
#include "Mmdeviceapi.h"
#include "HResultFailException.h"
#include "AudioDevice.h"

using namespace std;

namespace AudioDeviceApi
{

	struct IDComparison
	{
		bool operator()(wstring ws1, wstring ws2) const
		{
			return ws1 < ws2;
		}
	};
	
	typedef tr1::shared_ptr<AudioDevice> AudioDevicePtr;
	typedef pair<wstring, AudioDevicePtr> IDAudioDevicePair;
	typedef map<wstring, AudioDevicePtr, IDComparison> AudioDeviceMap;
	typedef tr1::shared_ptr<AudioDeviceMap>AudioDeviceMapPtr;
	//typedef tr1::shared_ptr<IMMDeviceEnumerator> IMMDeviceEnumeratorPtr;
	//typedef tr1::shared_ptr<IMMDeviceCollection> IMMDeviceCollectionPtr;


	class AudioDeviceController
	{

	public:
		AudioDeviceController(EDataFlow dataFlow = eRender, DWORD dwStateMask = DEVICE_STATE_ACTIVE);
		~AudioDeviceController();
		int GetCount();
		AudioDeviceMapPtr GetDevices();
		AudioDevicePtr GetDefaultDevice(ERole role = eMultimedia, EDataFlow dataFlow = (EDataFlow)0);
		void SetDefaultDevice(AudioDevicePtr audioDevice, ERole role);
		void SetDefaultDevice(wstring deviceID, ERole role);
		void SetDefaultDevice(LPCWSTR deviceID, ERole role);

		static bool testHResult(HRESULT hr)
		{
			if (!SUCCEEDED(hr)) {
				throw HResultFailException(hr);
			} else {
				return true;
			}
		};

		static DeviceType::Enum DataFlow2DeviceType(EDataFlow dataflow)
		{
			switch (dataflow) {
			case eRender:
				return DeviceType::PLAYBACK;
			case eCapture:
				return DeviceType::RECORDING;
			default:
				return DeviceType::UNKNOWN;
			}
		}
		

	protected:
		//HRESULT hr;
		IMMDeviceEnumerator* pDeviceEnum;
		IMMDeviceCollection* pDeviceCollection;
		EDataFlow dataFlow;
		AudioDeviceMapPtr pDeviceMap;
		AudioDevicePtr pDefaultDevice;

		IMMDeviceEnumerator* getpDeviceEnum();
		IMMDeviceCollection* getpDeviceCollection();
		AudioDeviceMapPtr getpDeviceMap();
		AudioDevicePtr getpDefaultDevice();

	};

}

#endif