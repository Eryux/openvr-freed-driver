# OpenVR Free-D Driver

OpenVR Free-D is a driver library for OpenVR system that simulate a virtual tracker in VR environnement using Free-D stream coordinates.


### What's free-d ?

Free-D is a protocol based on UDP packet for transfering 3D spatial position in "real-time" between applications. It's mostly used for camera tracking. Free-D can also include additional data like focus distance or zoom.

---

## Installation

**Currently OpenVR Free-D driver has only been tested on Windows x64 system, it should work on Linux but you will need to re-build the driver for.**

- Download latest (pre-)release in [release section](https://github.com/Eryux/openvr-freed-driver/releases).

- Extract the file in SteamVR driver folder: `<steam install path>\steamapps\common\SteamVR\drivers`.
  
If installation is correctly done, you should see Free-D icon beside your other SteamVR devices icons after launching SteamVR.

---

## Usage

Before using OpenVR Free-D driver you need to specify Free-D configuration and you probably want to make your Free-D and SteamVR world match together. You can do both by editing the main driver config file or by using the driver tool included.


### Config file

The config file is located inside freed driver folder at `<driver path>\resources\settings\default.vrsettings`. It's a JSON file that you can open it with the text editor of your choice.

Inside the file you should have something similar than below -

```
{
    "driver_freed": {
        "freed_address": "127.0.0.1",
        "freed_id": 1,
        "freed_port": 1337,
        "freed_shiftangle": 32768.0,
        "freed_shiftpos": 64.0,
        "offset_pitch": 0.0,
        "offset_roll": 0.0,
        "offset_x": 0.0,
        "offset_y": 0.0,
        "offset_yaw": 0.0,
        "offset_z": 0.0,
        "steam_config_path": "C:\\Program Files (x86)\\Steam\\config",
        "universe_x": 0,
        "universe_y": 0,
        "universe_yaw": 0,
        "universe_z": 0
    }
}
```

--


`freed_address` - Free-D listening address. Make sure you can reach the address inside your network.

`freed_port` - Free-D listening port. Make sure the port is not already used by another application.

`freed_id` - Id of Free-D device.

`freed_shiftangle` - Angle shift from Free-D protocol.

`freed_shiftpos` - Position shift from Free-D protocol.

`offset_pitch|roll|yaw|x|y|z` - Offsets added to position/orientation of the virtual tracker. Offset calculation is performed after universe calculation so it will always use SteamVR world.

`universe_x|y|z|yaw` - Offsets to make Free-D and SteamVR world match. If you leave this empty the origin of the world for Free-D position will be the center of one of your station which is the default origin of an OpenVR world. To find wich values to setup you can calibrate your world using SteamVR calibration apps then retrieve calibration values by opening `<steam install path>\config\chaperone_info.vrchap` and locate `seated` values corresponding to your SteamVR universe.

`steam_config_path` - Steam configuration folder, by default located at `<steam install path\config`.


### Driver Tool

The driver tool is a small console utility that help you to configure OpenVR driver if you don't want to directly edit the config file. The utility is located at `<steam install path>\steamapps\common\SteamVR\drivers\freed\win64\freed_driver_tool.exe`.

To use the utility, open a terminal in utility folder then launch it `.\freed_driver_tool.exe`.

The first option will allow you to configure Free-D protocol.

The second option is for calibrate your Free-D world with your SteamVR universe. It will read the chaperone calibration. SteamVR need to be launched to successfully retrieve all information.

The last option is to calibrate your Free-D world using the position of an already active OpenVR device by setting the origin to Free-D world at the OpenVR device position.

---

## Build

### Requierements

- Visual Studio 2022+
- Visual C++ Toolset v143 (2022)


### Steps

- Clone the repository on your device

- Clone and initialize submodules (`git submodule init`)

- Open solution `openvr-freed-tracker.sln` with your Visual Studio.

- Select `Release` or `Debug` and `x64` on target platform.
  
- Right click on `Solution 'openvr-freed-tracker'` then click on `Generate solution`.

- Copy all files in `ext\openvr\bin\win64` to `driver\freed\bin\win64`.

At this point OpenVR Free-D driver is ready to be use, you will just need to install it by copying `driver\freed` folder to your SteamVR driver folder or by registering `driver` folder to OpenVR external driver folder using `vrpathreg.exe` located in `<steam install path>\steamapps\common\SteamVR\bin\win64`.

---

## Contributing

Contributing is always welcome. 
If you want to help on this project by contributing on its developement, by reporting bugs, errors or mistakes or simply by giving your feedback, use the [issues section](https://github.com/Eryux/openvr-freed-driver/issues). Before posting or doing anything be sure to read the CONTRIBUTING.md which include basic contribution guidelines.

---

## Credits

Nicolas C. (Eryux) - **Author**


This project also uses the following open source packages :

- [g-truc/glm](https://github.com/g-truc/glm)
- [nlohmann/json](https://github.com/nlohmann/json)
- [valvesoftware/openvr](https://github.com/ValveSoftware/openvr)

---

## License

MIT License