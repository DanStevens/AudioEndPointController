#include "AudioDevice.h"
#include "AudioDeviceController.h"
#include "Functiondiscoverykeys_devpkey.h"
#include <string>

using namespace std;

namespace AudioDeviceApi
{

	//AudioDevice::AudioDevice(LPCWSTR id)
	//	: id(wstring(id))
	//{
	//	
	//}

	//AudioDevice::AudioDevice(IMMDevice* pMMDevice)
	//{ 
	//	AudioDevice(pMMDevice, DeviceType::UNKNOWN);
	//}

	AudioDevice::AudioDevice(IMMDevice* pMMDevice, DeviceType::Enum type /*= DeviceType::UNKNOWN */)
		: type(type)
	{
		LPWSTR deviceID;
		testHResult(pMMDevice->GetId(&deviceID));
		id = wstring(deviceID);

		DWORD state;
		testHResult(pMMDevice->GetState(&state));
		this->state = (DeviceState::Enum) state;

		IPropertyStore *pStore;
		testHResult(pMMDevice->OpenPropertyStore(STGM_READ, &pStore));

		name = getDeviceProperty(pStore, PKEY_DeviceInterface_FriendlyName);
		description = getDeviceProperty(pStore, PKEY_Device_DeviceDesc);
	}

	AudioDevice::~AudioDevice()
	{

	}

	wstring AudioDevice::getDeviceProperty(IPropertyStore* pStore, const PROPERTYKEY key)
{
	PROPVARIANT prop;
	wstring result;
	PropVariantInit(&prop);
	try {
		testHResult(pStore->GetValue(key, &prop));
		result = wstring(prop.pwszVal);
	}
	catch (HResultFailException)
	{
		result = wstring (L"");
	}
	PropVariantClear(&prop);
	return result;
}

}