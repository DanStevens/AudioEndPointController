#pragma once
#ifndef _AUDIODEVICE_
#define _AUDIODEVICE_

#include <string>
#include "windows.h"
#include "Mmdeviceapi.h"
#include "HResultFailException.h"

using namespace std;

namespace AudioDeviceApi
{

	namespace DeviceState {
		enum Enum { UNKNOWN, ACTIVE, DISABLED, NOTPRESENT, UNPLUGGED };
	}
	namespace DeviceType {
		enum Enum { UNKNOWN, PLAYBACK, RECORDING };
	}
	
	class AudioDevice
	{

	public:
		
		//AudioDevice(LPCWSTR id);
		//AudioDevice(IMMDevice* pMMDevice);
		AudioDevice(IMMDevice* pMMDevice, DeviceType::Enum type = DeviceType::UNKNOWN);
		~AudioDevice();
		wstring GetID()
		{
			return id;
		}
		DeviceState::Enum GetState()
		{
			return state;
		}
		DeviceType::Enum GetType()
		{
			return type;
		}
		wstring GetName()
		{
			return name;
		}
		wstring GetDescription()
		{
			return description;
		}
		//void SetAsDefault();

		static bool testHResult(HRESULT hr)
		{
			if (!SUCCEEDED(hr)) {
				throw HResultFailException(hr);
			} else {
				return true;
			}
		}

		

	protected:
		wstring id;
		DeviceState::Enum state;
		DeviceType::Enum type;
		//int isDefault;
		wstring name;
		wstring description;

		static wstring getDeviceProperty(IPropertyStore* pStore, const PROPERTYKEY key);
	
	};

}

#endif
