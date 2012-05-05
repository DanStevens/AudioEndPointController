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
	
	/**
	 * Represents an Audio End-point Device on a Windows Vista or Windows 7 system. The class acts as a wrapper for
	 * an IMMDevice instance, providing more convienient access to the properties and status of the IMMDevice object.
	 * Instances of this class can exist indepentantly from the IMMDevice, as the original values are copied from the
	 * IMMDevice and are affectively cached; the properties of the AudioDevice can still be accessed after the 
	 * IMMDevice used to initialize it has been released.
	 *
	 * See http://msdn.microsoft.com/en-us/library/dd371395%28v=vs.85%29.aspx for more information in the IMMDevice
	 * interface.
	 */
	class AudioDevice
	{

	public:
		
		/**
		 * Constructs a new AudioDevice object from the given IMMDevice.
		 * \param ppMMDevice a pointer to a pMMDevice to create the AudioDevice object from.
		 * \param The type of AudioDevice being created. Note that it is not possible to infer the type of device
		 *        from IMMDevice object, hence this property.
		 */
		AudioDevice(IMMDevice* pMMDevice, DeviceType::Enum type = DeviceType::UNKNOWN);
		
		//! Deconstructor
		~AudioDevice();
		
		//! Returns the unique Endpoint ID string for the AudioDevice (see http://msdn.microsoft.com/en-us/library/dd370837%28v=vs.85%29.aspx).
		wstring GetID()
		{
			return id;
		}

		//! Returns the state of the AudioDevice, if known.
		DeviceState::Enum GetState()
		{
			return state;
		}

		//! Returns the type of the AudioDevice, if known.
		DeviceType::Enum GetType()
		{
			return type;
		}

		//! Returns the name of the AudioDevice (equivalent to the PKEY_DeviceInterface_FriendlyName property of the 
		//! IMMDevice).
		wstring GetName()
		{
			return name;
		}

		//! Returns the name of the AudioDevice (equivalent to the PKEY_Device_DeviceDesc property of the 
		//! IMMDevice).
		wstring GetDescription()
		{
			return description;
		}

		/**
		 * Tests the passed HRESULT and throws an HResultFailException if it is anything except success.
		 * \param hr The HRESULT to test.
		 * \return true if the HRESULT does not indicate a failure.
		 */
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
		wstring name;
		wstring description;

		static wstring getDeviceProperty(IPropertyStore* pStore, const PROPERTYKEY key);
	
	};

}

#endif
