#pragma once

#include <openvr_driver.h>
#include "TrackerDevice.h"

namespace freed_tracker_driver {
	class DriverProvider : public vr::IServerTrackedDeviceProvider
	{
		public:
			virtual vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext);
			virtual void Cleanup();
			virtual const char* const* GetInterfaceVersions() { return vr::k_InterfaceVersions; }
			virtual void RunFrame();
			virtual bool ShouldBlockStandbyMode() { return false; }
			virtual void EnterStandby() {}
			virtual void LeaveStandby() {}

		private:
			TrackerDevice* m_trackerDevice = nullptr;
	};
}