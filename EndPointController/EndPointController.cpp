#include <stdio.h>
#include <wchar.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "windows.h"
#include "Mmdeviceapi.h"
#include "PolicyConfig.h"
#include "Propidl.h"
#include "Functiondiscoverykeys_devpkey.h"

// Format default string for outputting a device entry. The following parameters will be used in the following order:
// Index, Device Friendly Name
#define DEVICE_OUTPUT_FORMAT L"Audio Device %d: %ws"
#define DEVICE_DETAILED_FORMAT L"Device Index: %d, Name: %ws, State: %d, Default: %d, Descriptions: %ws, Interface Name: %ws, Device ID: %ws\n"

typedef struct TGlobalState
{
    HRESULT hr;
    int option;
    IMMDeviceEnumerator *pEnum;
    IMMDeviceCollection *pDevices;
    LPWSTR strDefaultDeviceID;
    IMMDevice *pCurrentDevice;
    LPCWSTR pDeviceFormatStr;
    int deviceStateFilter;
} TGlobalState;

std::vector<std::pair<std::wstring, std::wstring>> cachedOutputDevices; // Stores (Device Name, Device ID) for output devices
std::vector<std::pair<std::wstring, std::wstring>> cachedInputDevices;  // Stores (Device Name, Device ID) for input devices

// Function declarations
void createDeviceEnumerator(TGlobalState* state, bool isOutput);
void enumerateDevices(TGlobalState* state, bool isOutput);
HRESULT printDeviceInfo(IMMDevice* pDevice, int index, LPCWSTR outFormat, LPWSTR strDefaultDeviceID);
std::wstring getDeviceProperty(IPropertyStore* pStore, const PROPERTYKEY key);
HRESULT SetDefaultAudioPlaybackDevice(LPCWSTR devID);
HRESULT SetDefaultAudioCaptureDevice(LPCWSTR devID);
void invalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, 
    unsigned int line, uintptr_t pReserved);
void cacheDeviceList(bool isOutput);
void loadDeviceCache(bool isOutput);
HRESULT setDefaultDeviceFromCache(int deviceIndex, bool isOutput);
LPWSTR getDefaultDeviceID(EDataFlow dataFlow);

// Main function
int _tmain(int argc, LPCWSTR argv[])
{
    TGlobalState state = { 0 };
    bool isOutput = true; // Default to output devices

    // Initialize COM library
    state.hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(state.hr))
    {
        return state.hr;
    }

    // Process command line arguments
    state.option = -1; // Default is no option
    state.pDeviceFormatStr = DEVICE_OUTPUT_FORMAT; // Default to simple format
    state.deviceStateFilter = DEVICE_STATE_ACTIVE;

    for (int i = 1; i < argc; i++) 
    {
        if (wcscmp(argv[i], _T("--help")) == 0)
        {
            wprintf_s(_T("Lists active audio end-point devices (playback or capture) or sets default audio end-point\n"));
            wprintf_s(_T("device.\n\n"));
            wprintf_s(_T("USAGE\n"));
            wprintf_s(_T("  EndPointController.exe [--input | --output] [-a] [-f format_str]  Lists audio end-point devices\n"));
            wprintf_s(_T("  EndPointController.exe device_index [--input | --output]         Sets the default device\n"));
            wprintf_s(_T("\n"));
            wprintf_s(_T("OPTIONS\n"));
            wprintf_s(_T("  --input         Target input devices (microphones).\n"));
            wprintf_s(_T("  --output        Target output devices (speakers/headphones) [Default].\n"));
            wprintf_s(_T("  -a              Display all devices, rather than just active devices.\n"));
            wprintf_s(_T("  -f format_str   Outputs the details of each device using the given format string.\n"));
            exit(0);
        }
        else if (wcscmp(argv[i], _T("-a")) == 0)
        {
            state.deviceStateFilter = DEVICE_STATEMASK_ALL;
        }
        else if (wcscmp(argv[i], _T("-f")) == 0)
        {
            if ((argc - i) >= 2) {
                state.pDeviceFormatStr = argv[++i]; // Use the provided format string

                _set_invalid_parameter_handler(invalidParameterHandler);
                _CrtSetReportMode(_CRT_ASSERT, 0);
            }
            else
            {
                wprintf_s(_T("Missing format string"));
                exit(1);
            }
        }
        else if (wcscmp(argv[i], _T("--input")) == 0)
        {
            isOutput = false;
        }
        else if (wcscmp(argv[i], _T("--output")) == 0)
        {
            isOutput = true;
        }
        else if (isdigit(argv[i][0]))
        {
            state.option = _wtoi(argv[i]); // Capture the device index
        }
    }

    // Retrieve the correct default device ID based on input or output
    state.strDefaultDeviceID = getDefaultDeviceID(isOutput ? eRender : eCapture);

    // If setting a default device, load the cache and set it
    if (state.option != -1) 
    {
        loadDeviceCache(isOutput);  // Load cached device list
        state.hr = setDefaultDeviceFromCache(state.option - 1, isOutput);
    }
    else 
    {
        // If listing devices, enumerate them
        createDeviceEnumerator(&state, isOutput);
    }

    // Uninitialize COM library
    CoUninitialize();

    return state.hr;
}

// Retrieve the default audio device ID for comparison
LPWSTR getDefaultDeviceID(EDataFlow dataFlow)
{
    IMMDeviceEnumerator* pEnum = NULL;
    IMMDevice* pDevice = NULL;
    LPWSTR strDefaultDeviceID = NULL;

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
    if (SUCCEEDED(hr))
    {
        hr = pEnum->GetDefaultAudioEndpoint(dataFlow, eConsole, &pDevice);
        if (SUCCEEDED(hr))
        {
            pDevice->GetId(&strDefaultDeviceID);
            pDevice->Release();
        }
        pEnum->Release();
    }
    return strDefaultDeviceID;
}

// Load the device list from a file for caching
void loadDeviceCache(bool isOutput)
{
    std::wifstream inFile(isOutput ? L"output_device_cache.txt" : L"input_device_cache.txt");
    std::wstring line;
    while (std::getline(inFile, line))
    {
        size_t delimiterPos = line.find(L"|");
        if (delimiterPos != std::wstring::npos)
        {
            std::wstring name = line.substr(0, delimiterPos);
            std::wstring id = line.substr(delimiterPos + 1);
            if (isOutput)
                cachedOutputDevices.push_back(std::make_pair(name, id));
            else
                cachedInputDevices.push_back(std::make_pair(name, id));
        }
    }
    inFile.close();
}

// Create a multimedia device enumerator (only for listing devices)
void createDeviceEnumerator(TGlobalState* state, bool isOutput)
{
    state->pEnum = NULL;
    state->hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
        (void**)&state->pEnum);
    if (SUCCEEDED(state->hr))
    {
        EDataFlow dataFlow = isOutput ? eRender : eCapture;
        state->hr = state->pEnum->EnumAudioEndpoints(dataFlow, state->deviceStateFilter, &state->pDevices);
        if (SUCCEEDED(state->hr))
        {
            enumerateDevices(state, isOutput);
        }
        state->pEnum->Release();
    }
}

// Enumerate the devices (input or output) for listing
void enumerateDevices(TGlobalState* state, bool isOutput)
{
    UINT count;
    state->pDevices->GetCount(&count);

    for (UINT i = 0; i < count; i++)
    {
        state->hr = state->pDevices->Item(i, &state->pCurrentDevice);
        if (SUCCEEDED(state->hr))
        {
            printDeviceInfo(state->pCurrentDevice, i + 1, state->pDeviceFormatStr, state->strDefaultDeviceID);
            state->pCurrentDevice->Release();
        }
    }
}

// Print device info based on the format
HRESULT printDeviceInfo(IMMDevice* pDevice, int index, LPCWSTR outFormat, LPWSTR strDefaultDeviceID)
{
    LPWSTR strID = NULL;
    HRESULT hr = pDevice->GetId(&strID);
    if (!SUCCEEDED(hr))
    {
        return hr;
    }

    int deviceDefault = (strDefaultDeviceID != nullptr && wcscmp(strDefaultDeviceID, strID) == 0);

    DWORD dwState;
    hr = pDevice->GetState(&dwState);
    if (!SUCCEEDED(hr))
    {
        return hr;
    }

    IPropertyStore* pStore = NULL;
    hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
    if (SUCCEEDED(hr))
    {
        std::wstring friendlyName = getDeviceProperty(pStore, PKEY_Device_FriendlyName);
        std::wstring description = getDeviceProperty(pStore, PKEY_Device_DeviceDesc);
        std::wstring interfaceName = getDeviceProperty(pStore, PKEY_DeviceInterface_FriendlyName);

        wprintf_s(outFormat, index, friendlyName.c_str(), dwState, deviceDefault, description.c_str(), interfaceName.c_str(), strID); // Print device info
        wprintf_s(L"\n");

        pStore->Release();
    }

    return hr;
}

// Cache the device list to a file
void cacheDeviceList(bool isOutput)
{
    std::wofstream outFile(isOutput ? L"output_device_cache.txt" : L"input_device_cache.txt");
    const auto& cache = isOutput ? cachedOutputDevices : cachedInputDevices;

    for (const auto& device : cache)
    {
        outFile << device.first << L"|" << device.second << std::endl;
    }
    outFile.close();
}

// Set default device from the cache
HRESULT setDefaultDeviceFromCache(int deviceIndex, bool isOutput)
{
    const auto& cache = isOutput ? cachedOutputDevices : cachedInputDevices;

    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(cache.size()))
    {
        return E_INVALIDARG;
    }

    std::wstring deviceID = cache[deviceIndex].second;
    return isOutput ? SetDefaultAudioPlaybackDevice(deviceID.c_str()) : SetDefaultAudioCaptureDevice(deviceID.c_str());
}

// Retrieve a property from the device's property store
std::wstring getDeviceProperty(IPropertyStore* pStore, const PROPERTYKEY key)
{
    PROPVARIANT prop;
    PropVariantInit(&prop);
    HRESULT hr = pStore->GetValue(key, &prop);
    if (SUCCEEDED(hr))
    {
        std::wstring result(prop.pwszVal);
        PropVariantClear(&prop);
        return result;
    }
    return std::wstring(L"");
}

HRESULT SetDefaultAudioPlaybackDevice(LPCWSTR devID)
{
    IPolicyConfigVista *pPolicyConfig;
    ERole reserved = eConsole;

    HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient), 
        NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfig);
    if (SUCCEEDED(hr))
    {
        hr = pPolicyConfig->SetDefaultEndpoint(devID, reserved);
        pPolicyConfig->Release();
    }
    return hr;
}

HRESULT SetDefaultAudioCaptureDevice(LPCWSTR devID)
{
    IPolicyConfigVista *pPolicyConfig;
    HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient), 
        NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfig);

    if (SUCCEEDED(hr))
    {
        hr = pPolicyConfig->SetDefaultEndpoint(devID, eConsole);
        hr = pPolicyConfig->SetDefaultEndpoint(devID, eMultimedia);
        hr = pPolicyConfig->SetDefaultEndpoint(devID, eCommunications);

        pPolicyConfig->Release();
    }

    return hr;
}

void invalidParameterHandler(const wchar_t* expression,
   const wchar_t* function, 
   const wchar_t* file, 
   unsigned int line, 
   uintptr_t pReserved)
{
   exit(1);
}
