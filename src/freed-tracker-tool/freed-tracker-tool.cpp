#include <filesystem>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <openvr.h>

#ifdef _WIN32
const char* _setting_file = "..\\..\\resources\\settings\\default.vrsettings";
#else
const char* _setting_file = "../../resources/settings/default.vrsettings";
#endif


// Retrieve executable path -
// https://stackoverflow.com/questions/1528298/get-path-of-executable
#ifdef _WIN32
#include <windows.h>
#include <libloaderapi.h>
#else
#include <limits.h>
#include <unistd.h>
#endif


std::filesystem::path getexepath()
{
#ifdef _WIN32
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return path;
#else
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::string(result, (count > 0) ? count : 0);
#endif
}


using json = nlohmann::json;

json read_config_file(const char* filepath)
{
    std::ifstream f(filepath);

    if (f.is_open())
    {
        return json::parse(f);
    }
    
    return nullptr;
}

bool write_config_file(json* data, const char* filepath)
{
    std::ofstream f(filepath);

    if (f.is_open())
    {
        f << data->dump(4);
        return true;
    }

    return false;
}


void action_configure_freed()
{
    std::string freed_addr;
    std::cout << "Listenning address: ";
    std::cin >> freed_addr;

    int freed_port;
    std::cout << "Listenning port: ";
    std::cin >> freed_port;

    int freed_id = 1;
    std::cout << "Id: ";
    std::cin >> freed_id;

    float freed_shiftpos = 64.0f;
    std::cout << "Position shift (64 by default): ";
    std::cin >> freed_shiftpos;

    float freed_shiftangle = 32768.0f;
    std::cout << "Angle shift (32768 by default): ";
    std::cin >> freed_shiftangle;


    std::filesystem::path setting_file = getexepath().remove_filename().append(_setting_file);

    json setting_data = read_config_file(setting_file.generic_string().c_str());

    if (setting_data != nullptr)
    {
        setting_data["driver_freed"]["freed_address"] = freed_addr;
        setting_data["driver_freed"]["freed_port"] = freed_port;
        setting_data["driver_freed"]["freed_shiftpos"] = freed_shiftpos;
        setting_data["driver_freed"]["freed_shiftangle"] = freed_shiftangle;
        setting_data["driver_freed"]["freed_id"] = freed_id;

        if (write_config_file(&setting_data, setting_file.generic_string().c_str()))
        {
            std::cout << "Configuration saved. Please restart SteamVR if its running." << std::endl << std::endl;
            return;
        }
    }

    std::cout << "Unable to save configuration." << std::endl << std::endl;
}


void action_calibrate_index()
{
    // 1. Use steamvr universe (HMD required)
    // 2. Custom calibration
}

void action_calibrate_steamvr()
{
    // 1. Get HMD and HMD universe ID
    // 2. Open SteamVR chaperonne config file
    // 3. Search for universe Id corresponding
    // 4. Open driver setting and copy data to driver setting universe
    // 5. Restart steamVR
}

void action_calibrate_custom()
{
    // 1. List all tracked device
    // 2. Display info and wait for Enter
    // 3. Pick save data of tracked device
    // 4. Open driver setting and copy data to driver setting universe
    // 5. Restart steamVR
}


int main()
{
    char a;
    do {
        std::cout << "Free-d Tracker Driver - Tools" << std::endl;
        std::cout << "=============================" << std::endl;
        std::cout << "1. Configure Free-D" << std::endl;
        std::cout << "2. Calibrate" << std::endl;
        std::cout << "0. Quit" << std::endl;

        std::cout << std::endl << "Choices (0-2): ";
        std::cin >> a;

        if (a == '1')
        {
            action_configure_freed();
        }
    } while (a != '0');

    std::cout << "Bye." << std::endl;
    return 0;
}


/*int main()
{
    vr::HmdError vr_err = vr::HmdError::VRInitError_None;

    vr::IVRSystem* vr_sys = vr::VR_Init(&vr_err, vr::EVRApplicationType::VRApplication_Background);

    if (vr_err != vr::HmdError::VRInitError_Init_NoServerForBackgroundApp)
    {
        std::cout << "SteamVR is running" << std::endl;

        std::cout << "-" << std::endl;

        // Get HMD
        unsigned int hmdDeviceIndex = 0;
        while (vr_sys->GetTrackedDeviceClass(hmdDeviceIndex) != vr::TrackedDeviceClass::TrackedDeviceClass_HMD && hmdDeviceIndex < vr::k_unMaxTrackedDeviceCount)
        {
            hmdDeviceIndex++;
        }

        if (vr_sys->GetTrackedDeviceClass(hmdDeviceIndex) == vr::TrackedDeviceClass::TrackedDeviceClass_HMD)
        {
            std::cout << "HMD index : " << hmdDeviceIndex << std::endl;

            vr::TrackedPropertyError err;
            unsigned long hmdUniverseId = vr_sys->GetUint64TrackedDeviceProperty(hmdDeviceIndex, vr::Prop_CurrentUniverseId_Uint64, &err);
            if (err == vr::TrackedPropertyError::TrackedProp_Success)
            {
                std::cout << "HMD universe id : " << hmdUniverseId << std::endl;
            }

            char* hmdChaperonePath = new char[254];
            vr_sys->GetStringTrackedDeviceProperty(hmdDeviceIndex, vr::Prop_DriverProvidedChaperonePath_String, hmdChaperonePath, 254, &err);
            if (err == vr::TrackedPropertyError::TrackedProp_Success)
            {
                std::cout << "HMD chaperone : " << hmdChaperonePath << std::endl;
            }

            char* installPath = new char[254];
            vr_sys->GetStringTrackedDeviceProperty(hmdDeviceIndex, vr::Prop_InstallPath_String, installPath, 254, &err);
            if (err == vr::TrackedPropertyError::TrackedProp_Success)
            {
                std::cout << "HMD install path : " << installPath << std::endl;
            }

            char* configPath = new char[254];
            vr_sys->GetStringTrackedDeviceProperty(hmdDeviceIndex, vr::Prop_UserConfigPath_String, configPath, 254, &err);
            if (err == vr::TrackedPropertyError::TrackedProp_Success)
            {
                std::cout << "HMD chaperone : " << configPath << std::endl;
            }
        }
        else
        {
            std::cout << "No HMD" << std::endl;
        }
    }
    else
    {
        std::cout << "SteamVR is not running, abort." << std::endl;
    }

    vr::VR_Shutdown();
}*/