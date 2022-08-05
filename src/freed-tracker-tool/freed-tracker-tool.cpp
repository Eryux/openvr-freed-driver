#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>

#include "nlohmann/json.hpp"
#include "openvr.h"
#include "glm.hpp"
#include "gtc/quaternion.hpp"
#include "gtx/matrix_decompose.hpp"

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


vr::IVRSystem* _vr;


void action_configure_freed()
{
    std::string freed_addr;
    std::cout << "Listenning address: ";
    std::cin >> freed_addr;
    std::cin.clear(); std::cin.ignore(INT_MAX, '\n');

    int freed_port;
    std::cout << "Listenning port: ";
    std::cin >> freed_port;
    std::cin.clear(); std::cin.ignore(INT_MAX, '\n');

    int freed_id = 1;
    std::cout << "Id: ";
    std::cin >> freed_id;
    std::cin.clear(); std::cin.ignore(INT_MAX, '\n');

    float freed_shiftpos = 64.0f;
    std::cout << "Position shift (64 by default): ";
    std::cin >> freed_shiftpos;
    std::cin.clear(); std::cin.ignore(INT_MAX, '\n');

    float freed_shiftangle = 32768.0f;
    std::cout << "Angle shift (32768 by default): ";
    std::cin >> freed_shiftangle;
    std::cin.clear(); std::cin.ignore(INT_MAX, '\n');


    std::filesystem::path setting_file = getexepath().remove_filename().append(_setting_file);

    json setting = read_config_file(setting_file.generic_string().c_str());

    if (setting != nullptr)
    {
        setting["driver_freed"]["freed_address"] = freed_addr;
        setting["driver_freed"]["freed_port"] = freed_port;
        setting["driver_freed"]["freed_shiftpos"] = freed_shiftpos;
        setting["driver_freed"]["freed_shiftangle"] = freed_shiftangle;
        setting["driver_freed"]["freed_id"] = freed_id;

        if (write_config_file(&setting, setting_file.generic_string().c_str()))
        {
            std::cout << "Configuration saved. Please restart SteamVR if its running." << std::endl << std::endl;
            return;
        }
    }

    std::cout << "Unable to save configuration." << std::endl;
}


void action_calibrate_steamvr()
{
    std::filesystem::path setting_file = getexepath().remove_filename().append(_setting_file);

    json setting = read_config_file(setting_file.generic_string().c_str());

    if (setting == nullptr)
    {
        std::cout << "Unable to read driver configuration file." << std::endl;
        return;
    }


    std::string steam_config_dir = setting["driver_freed"]["steam_config_path"];
    
    char a = 'n';

    do {
        if (steam_config_dir == "")
        {
            std::cout << "Enter your Steam config directory: ";
            std::getline(std::cin, steam_config_dir);
        }

        std::cout << "Your Steam config directory is " << steam_config_dir << " (y/n): ";
        a = std::getchar();
        std::cin.clear(); std::cin.ignore(INT_MAX, '\n');
    } while (a != 'y');

    setting["driver_freed"]["steam_config_path"] = steam_config_dir;

    
    // Find HMD device index
    uint16_t hmd_device_index = 0;
    while (_vr->GetTrackedDeviceClass(hmd_device_index) != vr::TrackedDeviceClass::TrackedDeviceClass_HMD 
        && hmd_device_index < vr::k_unMaxTrackedDeviceCount)
    {
        hmd_device_index++;
    }

    if (_vr->GetTrackedDeviceClass(hmd_device_index) != vr::TrackedDeviceClass::TrackedDeviceClass_HMD) 
    {
        std::cout << "No HMD found." << std::endl;
        return;
    }


    // Get current universe id used by HMD
    vr::TrackedPropertyError err_prop;
    uint64_t universe_id = _vr->GetUint64TrackedDeviceProperty(hmd_device_index, vr::Prop_CurrentUniverseId_Uint64, &err_prop);
    if (err_prop != vr::TrackedPropertyError::TrackedProp_Success)
    {
        std::cout << "No HMD universe found." << std::endl;
        return;
    }
    std::string universe_id_str = std::to_string(universe_id);


    // Open chaperone config file and search for calibration setting corresponding to HMD universe
    std::filesystem::path steam_chaperone_file = std::filesystem::path(steam_config_dir).append("chaperone_info.vrchap");
    
    json chaperone_setting = read_config_file(steam_chaperone_file.generic_string().c_str());

    if (chaperone_setting == nullptr)
    {
        std::cout << "Unable to open chaperone_info.vrchap in " << steam_config_dir << "." << std::endl;
        return;
    }

    json chaperone_universe = nullptr;
    for (auto universe : chaperone_setting["universes"])
    {
        if (universe["universeID"] == universe_id_str)
        {
            chaperone_universe = universe;
            break;
        }
    }

    if (chaperone_universe == nullptr)
    {
        std::cout << "No SteamVR calibration found for current universe." << std::endl;
        return;
    }


    // Apply settings
    setting["driver_freed"]["universe_x"] = chaperone_universe["standing"]["translation"][0];
    setting["driver_freed"]["universe_y"] = chaperone_universe["standing"]["translation"][1];
    setting["driver_freed"]["universe_z"] = chaperone_universe["standing"]["translation"][2];
    setting["driver_freed"]["universe_yaw"] = chaperone_universe["standing"]["yaw"];


    if (write_config_file(&setting, setting_file.generic_string().c_str()))
    {
        std::cout << "Calibration saved. Please restart SteamVR." << std::endl;
        return;
    }
}


struct vrdevice_t
{
    uint16_t index;
    char* name;
    char* serial;
    vr::ETrackedDeviceClass type;
};

void action_calibrate_custom()
{
    // Fetch and list all availables tracked devices
    std::vector<vrdevice_t> vr_devices;

    uint16_t device_index = 0;
    while (device_index < vr::k_unMaxTrackedDeviceCount)
    {
        vr::ETrackedDeviceClass device_type = _vr->GetTrackedDeviceClass(device_index);

        if (device_type != vr::ETrackedDeviceClass::TrackedDeviceClass_Invalid && device_type != vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference)
        {
            vrdevice_t device = vrdevice_t();
            device.index = device_index;
            device.type = device_type;

            vr::TrackedPropertyError err_prop;

            device.name = new char[256];
            _vr->GetStringTrackedDeviceProperty(device_index, vr::Prop_ModelNumber_String, device.name, 256, &err_prop);

            device.serial = new char[256];
            _vr->GetStringTrackedDeviceProperty(device_index, vr::Prop_SerialNumber_String, device.serial, 256, &err_prop);

            vr_devices.push_back(device);
        }

        device_index++;
    }


    std::cout << std::endl << "Available tracked device(s):" << std::endl;

    for (int i = 0; i < vr_devices.size(); ++i)
    {
        std::cout << i << ". " << vr_devices[i].name << " (" << vr_devices[i].index << ") [" << vr_devices[i].serial << "]" << std::endl;
    }

    std::cout << std::endl << "Which tracked device you want to use for calibration? Choice: ";

    int device_list_index = 0;
    std::cin >> device_list_index;
    std::cin.clear(); std::cin.ignore(INT_MAX, '\n');

    if (device_list_index < 0 || device_list_index > vr_devices.size())
    {
        std::cout << "Incorrect device number." << std::endl;
        return;
    }


    std::cout << "Place your device on the ground where you want the origin of your universe." << std::endl;
    std::cout << "Press ENTER when it is done." << std::endl;

    
    // Retrieve raw device's position
    vr::TrackedDevicePose_t device_pose = vr::TrackedDevicePose_t();
    vr::VREvent_t device_event = vr::VREvent_t();

    do
    {
        _vr->PollNextEventWithPose(vr::ETrackingUniverseOrigin::TrackingUniverseRawAndUncalibrated, &device_event, sizeof(vr::VREvent_t), &device_pose);
    } while (device_event.trackedDeviceIndex != vr_devices[device_list_index].index);


    glm::mat4 pose_mat = glm::mat4(1.0f);
    pose_mat[0][0] = device_pose.mDeviceToAbsoluteTracking.m[0][0]; 
    pose_mat[0][1] = device_pose.mDeviceToAbsoluteTracking.m[1][0]; 
    pose_mat[0][2] = device_pose.mDeviceToAbsoluteTracking.m[2][0];

    pose_mat[1][0] = device_pose.mDeviceToAbsoluteTracking.m[0][1]; 
    pose_mat[1][1] = device_pose.mDeviceToAbsoluteTracking.m[2][1]; 
    pose_mat[1][2] = device_pose.mDeviceToAbsoluteTracking.m[2][1];

    pose_mat[2][0] = device_pose.mDeviceToAbsoluteTracking.m[0][2]; 
    pose_mat[2][1] = device_pose.mDeviceToAbsoluteTracking.m[1][2]; 
    pose_mat[2][2] = device_pose.mDeviceToAbsoluteTracking.m[2][2];

    pose_mat[3][0] = device_pose.mDeviceToAbsoluteTracking.m[0][3];
    pose_mat[3][1] = device_pose.mDeviceToAbsoluteTracking.m[1][3];
    pose_mat[3][2] = device_pose.mDeviceToAbsoluteTracking.m[2][3];


    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(pose_mat, scale, rotation, translation, skew, perspective);
    glm::vec3 rotation_euler = glm::eulerAngles(rotation);

    
    // Save device's position as universe
    std::filesystem::path setting_file = getexepath().remove_filename().append(_setting_file);

    json setting = read_config_file(setting_file.generic_string().c_str());

    if (setting == nullptr)
    {
        std::cout << "Unable to read driver configuration file." << std::endl;
        return;
    }

    setting["driver_freed"]["universe_x"] = translation.x * -1.f;
    setting["driver_freed"]["universe_y"] = translation.y * -1.f;
    setting["driver_freed"]["universe_z"] = translation.z * -1.f;
    setting["driver_freed"]["universe_yaw"] = rotation_euler.y - glm::radians(180.0f);

    if (write_config_file(&setting, setting_file.generic_string().c_str()))
    {
        std::cout << "Calibration saved. Please restart SteamVR." << std::endl;
        return;
    }
}


int main()
{
    // OpenVR init
    vr::HmdError err_vr = vr::HmdError::VRInitError_None;

    _vr = vr::VR_Init(&err_vr, vr::EVRApplicationType::VRApplication_Background);
    if (err_vr == vr::HmdError::VRInitError_Init_NoServerForBackgroundApp)
    {
        vr::VR_Shutdown();
        _vr = nullptr;
    }


    char a;
    do {
        std::cout << std::endl << "Free-d Tracker Driver - Tools" << std::endl;
        std::cout << "=============================" << std::endl;
        std::cout << "1. Configure Free-D" << std::endl;
        std::cout << "2. Calibrate using SteamVR universe" << std::endl;
        std::cout << "3. Calibrate using tracked device" << std::endl;
        std::cout << "0. Quit" << std::endl;

        std::cout << std::endl << "Choices (0-3): ";

        a = std::getchar();
        std::cin.clear(); std::cin.ignore(INT_MAX, '\n');

        if (a == '1')
        {
            action_configure_freed();
        }
        else if (a == '2')
        {
            if (_vr == nullptr)
            {
                std::cout << "Error: SteamVR is not running." << std::endl;
            }
            else
            {
                action_calibrate_steamvr();
            }
        }
        else if (a == '3')
        {
            if (_vr == nullptr)
            {
                std::cout << "Error: SteamVR is not running." << std::endl;
            }
            else
            {
                action_calibrate_custom();
            }
        }
    } while (a != '0');

    std::cout << "Bye." << std::endl;
    return 0;
}