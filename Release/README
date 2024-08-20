# AudioEndPointController

A Windows command-line utility that allows you to list active audio end-points and set the default device. This tool includes optimized performance through device caching, enabling faster selection of default devices.

```
>EndPointController.exe --help
Lists active audio end-point playback or capture devices or sets the default audio end-point device.
```

## USAGE
```
EndPointController.exe [--input | --output] [-a] [-f format_str]  Lists audio end-point devices that are enabled.

EndPointController.exe device_index [--input | --output]         Sets the default device with the given index.
```

## OPTIONS
- `--input`          Target input devices (microphones).
- `--output`         Target output devices (speakers/headphones) [Default].
- `-a`               Display all devices, rather than just active devices.
- `-f format_str`    Outputs the details of each device using the given format string. If this parameter is omitted, the format string defaults to: `Audio Device %d: %ws`

  **Parameters passed to the 'printf' function are ordered as follows:**
  - Device index (int)
  - Device friendly name (wstring)
  - Device state (int)
  - Device default? (1 for true, 0 for false as int)
  - Device description (wstring)
  - Device interface friendly name (wstring)
  - Device ID (wstring)
```

Examples:

Get list of enabled output devices: `.\EndPointController.exe --output`
Get list of enabled input devices: `.\EndPointController.exe --input`
Set default output device: `.\EndPointController.exe 1`
Set default input device: `.\EndPointController.exe 1 --input`
Get device output details: `.\EndPointController.exe -f "Device Index: %d, Name: %ws, State: %d, Default: %d, Descriptions: %ws, Interface Name: %ws, Device ID: %ws"`
Get device output details: `.\EndPointController.exe -f "Device Index: %d, Name: %ws, State: %d, Default: %d, Descriptions: %ws, Interface Name: %ws, Device ID: %ws"`
Get device input details: `.\EndPointController.exe -f "Device Index: %d, Name: %ws, State: %d, Default: %d, Descriptions: %ws, Interface Name: %ws, Device ID: %ws" --input`