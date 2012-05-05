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
	
	//! A shared pointer to an AudioDevice object.
	typedef tr1::shared_ptr<AudioDevice> AudioDevicePtr;

	//! A shared key-value pair, with a wstring as the key and a AudioDevicePtr as the value. 
	typedef pair<wstring, AudioDevicePtr> IDAudioDevicePair;

	//! A map for storing a list of AudioDevicePtr, using the ID of the device as the key.
	typedef map<wstring, AudioDevicePtr, IDComparison> AudioDeviceMap;

	//! A shared pointer to an AudioDeviceMap object.
	typedef tr1::shared_ptr<AudioDeviceMap>AudioDeviceMapPtr;

	/**
	 * The AudioDeviceController class provides access Audio End-point Devices of the Windows Vista or Windows 7.
	 * Consumers can request a list of Audio Devices, and get/set the default audio device. By default,
	 * AudioDeviceController deals with Playback devices, but it can also be used to retrieve Recording devices and
	 * get/set the default Recording device by setting parameters of the appropriate function.
	 */
	class AudioDeviceController
	{

	public:
		
		/** 
		 * Constructs a new AudioDeviceController.
		 * \param dataFlow The data-flow direction for the endpoint devices to deal with. Use 'eRender' for Playback
		 *                 devices, 'eCapture' for Recording devices and 'eAll' for both Playbacka dn Recording
		 *                 devices.
		 * \param dwStateMask The state or states of the endpoints The caller should set this parameter to the bitwise
		 *                    OR of one or more of the following DEVICE_STATE_XXX constants:
		 *                      * DEVICE_STATE_ACTIVE
		 *                      * DEVICE_STATE_DISABLED
		 *                      * DEVICE_STATE_NOTPRESENT
		 *                      * DEVICE_STATE_UNPLUGGED
		 *                    The parameter can be set to DEVICE_STATEMASK_ALL to include all endpoints, regardless of
		 *                    state. See http://msdn.microsoft.com/en-us/library/dd371400%28v=vs.85%29.aspx for more
		 *                    information.
		 */ 
		AudioDeviceController(EDataFlow dataFlow = eRender, DWORD dwStateMask = DEVICE_STATE_ACTIVE);

		/**
		 * Deconstructor.
		 */
		~AudioDeviceController();
		
		/**
		 * Returns the number of devices that correspond with the criteria specified in the constructor.
		 */
		int GetCount();

		/**
		 * Returns a pointer to a map of AudioDevices that correspond with the criteria specifiedin the constructor,
		 * keyed by the ID of the device.
		 */
		AudioDeviceMapPtr GetDevices();

		/**
		 * Returns a pointer to an instance of AudioDevice that represents the default audio device.
		 * \param role The Role of the Audio Device. See http://msdn.microsoft.com/en-us/library/dd370813%28v=vs.85%29.aspx
		 *             The support for roles may depend on the version of Windows.
		 * \param dataFlow The data-flow direction of the endpoint device. If this is set to 0 (as per default), 
		 *                 the direction given in the constructor will be assumed. Note that eAll is not a valid value
		 *                 for this parameter and so 'eRender' is assumed in such case that it is specified here or in
		 *                 the constructor.
		 */
		AudioDevicePtr GetDefaultDevice(ERole role = eMultimedia, EDataFlow dataFlow = (EDataFlow)0);

		/**
		 * Sets the default Audio Endpoint device to the given AudioDevice.
		 * \param audioDevice a pointer to the AudioDevice to set as the default.
		 * \param the role to set default Audio Endpoint device for.
		 */
		void SetDefaultDevice(AudioDevicePtr audioDevice, ERole role = eMultimedia);

		/**
		 * Sets the default Audio Endpoint device to the device with the given ID, as a wstring
		 * \param audioDevice the ID of the device to set to default.
		 * \param the role to set default Audio Endpoint device for.
		 */
		void SetDefaultDevice(wstring deviceID, ERole role = eMultimedia);

		/**
		 * Sets the default Audio Endpoint device to the device with the given ID, as a LPCWSTR
		 * \param audioDevice the ID of the device to set to default.
		 * \param the role to set default Audio Endpoint device for.
		 */
		void SetDefaultDevice(LPCWSTR deviceID, ERole role = eMultimedia);

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
		};

		/**
		 * Converts a EDataFlow to the corresponding DeviceType::Enum.
		 * \param dataFlow the EDataFlow to convert to DeviceType::Enum
		 * \return a DeviceType::Enum, in accordance with the following:
		 *           * if dataFlow is eRender, return DeviceType::PLAYBACK
		 *           * if dataFlow is eCapture, return DeviceType::RECORDING
		 *           * if dataflow is anything else, return DeviceType::UNKNOWN
		 */
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