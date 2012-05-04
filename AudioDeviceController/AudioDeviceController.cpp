#include <map>
#include "AudioDeviceController.h"
#include "windows.h"
#include "Mmdeviceapi.h"
#include "PolicyConfig.h"
#include "AudioDevice.h"


namespace AudioDeviceApi
{
	
	AudioDeviceController::AudioDeviceController(EDataFlow dataFlow, DWORD dwStateMask)
		: pDeviceEnum(NULL), pDeviceCollection(NULL), dataFlow(dataFlow)
	{
		testHResult(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));

		testHResult(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
			(void**)&pDeviceEnum));

		testHResult(pDeviceEnum->EnumAudioEndpoints(dataFlow, dwStateMask, &pDeviceCollection));
	}

	AudioDeviceController::~AudioDeviceController()
	{
		getpDeviceCollection()->Release();
		getpDeviceEnum()->Release();
	}

	int AudioDeviceController::GetCount()
	{
		unsigned count;
		getpDeviceCollection()->GetCount(&count);
		return (int)count;
	}

	AudioDeviceMapPtr AudioDeviceController::GetDevices()
	{
		int count = GetCount();
		pDeviceMap = AudioDeviceMapPtr(new AudioDeviceMap());
		if (count > 0) {
			
			for (int i = 0; i < count; i++) {
				IMMDevice* pDevice;
				testHResult(pDeviceCollection->Item(i, &pDevice));
				AudioDevicePtr audioDevice(new AudioDevice(pDevice, DataFlow2DeviceType(dataFlow)));
				(*pDeviceMap)[audioDevice->GetID()] = audioDevice;
				//IDAudioDevicePair* pair = new IDAudioDevicePair(audioDevice->GetID(), audioDevice);
				//pDeviceMap->insert(*pair);
				pDevice->Release();
			}
			
		}
		return pDeviceMap;
	}

	AudioDevicePtr AudioDeviceController::GetDefaultDevice(ERole role /*= eMultimedia */, EDataFlow dataFlow /*= 0 */)
	{		
		// Use parameter for dataFlow, unless it is zero, in which case use object's dataflow
		EDataFlow df = ((int)dataFlow) ? dataFlow : this->dataFlow;
		
		// A dataFlow of 'eAll' is invalid, so assume 'eRender' if this has been specified.
		df = (df == eAll) ? eRender : df;
		
		IMMDevice* pDevice;
		testHResult(getpDeviceEnum()->GetDefaultAudioEndpoint(dataFlow, role, &pDevice));

		pDefaultDevice = AudioDevicePtr(new AudioDevice(pDevice, (DeviceType::Enum) dataFlow));
		pDevice->Release();
		return pDefaultDevice;
	}

	void AudioDeviceController::SetDefaultDevice(LPCWSTR deviceID, ERole role)
	{
			IPolicyConfigVista *pPolicyConfig;

			testHResult(CoCreateInstance(__uuidof(CPolicyConfigVistaClient), 
				NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfig));
			testHResult(pPolicyConfig->SetDefaultEndpoint(deviceID, role));
			pPolicyConfig->Release();
	}

	void AudioDeviceController::SetDefaultDevice(AudioDevicePtr audioDevice, ERole role)
	{
		SetDefaultDevice(audioDevice->GetID(), role);
	}

	void AudioDeviceController::SetDefaultDevice(wstring deviceID, ERole role)
	{
		SetDefaultDevice(deviceID.c_str(), role);
	}

	IMMDeviceEnumerator* AudioDeviceController::getpDeviceEnum()
	{
		if (pDeviceEnum != NULL)
		{
			return pDeviceEnum;
		} else {
			throw Exception("pDeviceEnum was NULL");
		}
	}

	IMMDeviceCollection* AudioDeviceController::getpDeviceCollection()
	{
		if (pDeviceCollection != NULL)
		{
			return pDeviceCollection;
		} else {
			throw Exception("pDeviceCollection was NULL");
		}
	}

	AudioDeviceMapPtr AudioDeviceController::getpDeviceMap()
	{
		if (pDeviceMap != NULL)
		{
			return pDeviceMap;
		} else {
			throw Exception("pDeviceMap was NULL");
		}
	}

	AudioDevicePtr AudioDeviceController::getpDefaultDevice()
	{
		if (pDefaultDevice != NULL)
		{
			return pDefaultDevice;
		} else {
			throw Exception("pDefaultDevice was NULL");
		}
	}

}
