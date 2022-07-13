#include "pch.h"

#include "DriverProvider.h"
#include <memory>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}


static std::shared_ptr<freed_tracker_driver::DriverProvider> g_driverProvider;


OPENVRDRIVER_DLL_EXPORT void* HmdDriverFactory(const char* pInterfaceName, int* pReturnCode)
{
	if (0 == strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName))
	{
        if (!g_driverProvider)
        {
            g_driverProvider = std::make_shared<freed_tracker_driver::DriverProvider>();
        }

		return g_driverProvider.get();
	}

    if (pReturnCode)
    {
		*pReturnCode = vr::VRInitError_Init_InterfaceNotFound;
    }

	return nullptr;
}
