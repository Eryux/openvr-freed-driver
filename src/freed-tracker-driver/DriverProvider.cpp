#include "pch.h"
#include "DriverProvider.h"
#include "Logger.h"

using namespace freed_tracker_driver;


vr::EVRInitError DriverProvider::Init(vr::IVRDriverContext* pDriverContext)
{
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

	InitLog(vr::VRDriverLog());

	m_trackerDevice = new TrackerDevice("FR33D", "Freed Virtual Tracker");
	vr::VRServerDriverHost()->TrackedDeviceAdded(m_trackerDevice->GetSerialNumber().c_str(), vr::TrackedDeviceClass_GenericTracker, m_trackerDevice);

	return vr::EVRInitError::VRInitError_None;
}


void DriverProvider::Cleanup()
{
	delete m_trackerDevice;
	m_trackerDevice = nullptr;

	CleanupLog();
}


void DriverProvider::RunFrame()
{
	if (m_trackerDevice != nullptr)
	{
		m_trackerDevice->Update();
	}
}