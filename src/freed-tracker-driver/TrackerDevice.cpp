#include "pch.h"
#include "TrackerDevice.h"
#include "Logger.h"


using namespace freed_tracker_driver;


TrackerDevice::TrackerDevice(std::string serialNumber, std::string modelNumber) : m_serialNumber(serialNumber), m_modelNumber(modelNumber)
{
	m_freed = new Freed();
}

TrackerDevice::~TrackerDevice()
{
	if (m_freed != nullptr)
	{
		delete m_freed;
	}
}


std::string TrackerDevice::GetSerialNumber()
{
	return this->m_serialNumber;
}


void TrackerDevice::Update()
{
	if (m_deviceIndex != vr::k_unTrackedDeviceIndexInvalid && m_initialized)
	{
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_deviceIndex, GetPose(), sizeof(vr::DriverPose_t));
	}
}


vr::DriverPose_t TrackerDevice::GetPose()
{
	vr::DriverPose_t pose =  TrackerDevice::DefaultPose(true, m_freed != nullptr && m_freed->IsListening());

	if (m_freed != nullptr)
	{
		FreedData_t currentPosition = m_freed->GetData();

		// Position
		pose.vecPosition[0] = currentPosition.position.x + m_position_offset.x;
		pose.vecPosition[1] = currentPosition.position.z + m_position_offset.y;
		pose.vecPosition[2] = -currentPosition.position.y + m_position_offset.z;

		// Rotation
		// In Free-d, pitch and roll are dependent from yaw
		glm::quat rot = glm::quat(glm::vec3(glm::radians(-currentPosition.rotation.x), 0.f, 0.f))
			* glm::quat(glm::radians(glm::vec3(0.f, currentPosition.rotation.y, currentPosition.rotation.z)));

		glm::vec3 euler = glm::eulerAngles(rot);

		rot = glm::quat(glm::vec3(euler.y, glm::radians(currentPosition.rotation.x), euler.z) * -1.f);
		rot *= m_rotation_offset;
		pose.qRotation.w = rot.w;
		pose.qRotation.x = rot.x;
		pose.qRotation.y = rot.y;
		pose.qRotation.z = rot.z;
	}

	return pose;
}


vr::EVRInitError TrackerDevice::Activate(uint32_t unObjectId)
{
	m_deviceIndex = unObjectId;
	m_propertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_deviceIndex);

	// Set device settings
	vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_ModelNumber_String, m_serialNumber.c_str());
	vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_RenderModelName_String, m_modelNumber.c_str());
	vr::VRProperties()->SetUint64Property(m_propertyContainer, vr::Prop_CurrentUniverseId_Uint64, 2);
	vr::VRProperties()->SetBoolProperty(m_propertyContainer,vr::Prop_IsOnDesktop_Bool, false);
	vr::VRProperties()->SetInt32Property(m_propertyContainer, vr::Prop_ControllerRoleHint_Int32, vr::ETrackedControllerRole::TrackedControllerRole_OptOut);
	vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_RenderModelName_String, "vr_tracker_vive_3_0");
	vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_InputProfilePath_String, "{freed}/input/freed_tracker_bindings.json");
	vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReady_String, "{freed}/icons/tracker_status_ready.png");
	vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceOff_String, "{freed}/icons/tracker_status_off.png");
	vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearching_String, "{freed}/icons/tracker_status_off.png");
	vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearchingAlert_String, "{freed}/icons/tracker_status_error.png");
	vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{freed}/icons/tracker_status_error.png");
	vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceNotReady_String, "{freed}/icons/tracker_status_off.png");
	vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, "{freed}/icons/tracker_status_off.png");
	vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceAlertLow_String, "{freed}/icons/tracker_status_ready_low.png");


	// Retrieve driver settings
	vr::EVRSettingsError serr = vr::EVRSettingsError::VRSettingsError_None;

	// Position offset
	float offset_x = vr::VRSettings()->GetFloat("driver_freed", "offset_x", &serr);
	if (serr != vr::EVRSettingsError::VRSettingsError_None) {
		offset_x = 0.0f;
	} m_position_offset.x = offset_x;

	float offset_y = vr::VRSettings()->GetFloat("driver_freed", "offset_y", &serr);
	if (serr != vr::EVRSettingsError::VRSettingsError_None) {
		offset_y = 0.0f;
	} m_position_offset.y = offset_y;

	float offset_z = vr::VRSettings()->GetFloat("driver_freed", "offset_z", &serr);
	if (serr != vr::EVRSettingsError::VRSettingsError_None) {
		offset_z = 0.0f;
	} m_position_offset.z = offset_z;


	// Rotation offset
	float offset_pitch = vr::VRSettings()->GetFloat("driver_freed", "offset_pitch", &serr);
	if (serr != vr::EVRSettingsError::VRSettingsError_None) {
		offset_pitch = 0.0f;
	}

	float offset_yaw = vr::VRSettings()->GetFloat("driver_freed", "offset_yaw", &serr);
	if (serr != vr::EVRSettingsError::VRSettingsError_None) {
		offset_yaw = 0.0f;
	}

	float offset_roll = vr::VRSettings()->GetFloat("driver_freed", "offset_roll", &serr);
	if (serr != vr::EVRSettingsError::VRSettingsError_None) {
		offset_roll = 0.0f;
	}

	m_rotation_offset = glm::quat(glm::radians(glm::vec3(offset_pitch, offset_yaw, offset_roll)));


	// Universe
	bool useSteamVRUniverse = vr::VRSettings()->GetBool("driver_freed", "use_steamvr_universe", &serr);
	if (serr != vr::EVRSettingsError::VRSettingsError_None) {
		useSteamVRUniverse = true;
	}

	if (useSteamVRUniverse)
	{
		// TODO
	}


	// Free-d settings
	char freedAddr[128] { 0 };
	vr::VRSettings()->GetString("driver_freed", "freed_address", freedAddr, 128, &serr);
	if (serr != vr::EVRSettingsError::VRSettingsError_None) {
		Log("Missing setting freed_address");
		return vr::EVRInitError::VRInitError_Init_SettingsInitFailed;
	}

	int freedPort = vr::VRSettings()->GetInt32("driver_freed", "freed_port", &serr);
	if (serr != vr::EVRSettingsError::VRSettingsError_None) {
		Log("Missing setting freed_port");
		return vr::EVRInitError::VRInitError_Init_SettingsInitFailed;
	}

	int freedId = vr::VRSettings()->GetInt32("driver_freed", "freed_id", &serr);
	if (serr != vr::EVRSettingsError::VRSettingsError_None) {
		freedId = 1;
	} m_freed->SetDeviceId(freedId);

	float freedPosShift = vr::VRSettings()->GetFloat("driver_freed", "freed_shiftpos", &serr);
	if (serr != vr::EVRSettingsError::VRSettingsError_None) {
		freedPosShift = 64.0f;
	} m_freed->SetPositionShift(freedPosShift);

	float freedAngleShift = vr::VRSettings()->GetFloat("driver_freed", "freed_shiftangle", &serr);
	if (serr != vr::EVRSettingsError::VRSettingsError_None) {
		freedAngleShift = 32768.0f;
	} m_freed->SetAngleShift(freedAngleShift);


	// Start Free-d interface
	bool r = m_freed->StartListening(freedAddr, freedPort);
	if (!r) {
		Log("Unable to listen interface, please check listen address and port.");
		return vr::EVRInitError::VRInitError_Init_SettingsInitFailed;
	}


	m_initialized = true;
	return vr::EVRInitError::VRInitError_None;
}


void TrackerDevice::Deactivate()
{
	this->m_deviceIndex = vr::k_unTrackedDeviceIndexInvalid;

	if (m_freed != nullptr && m_freed->IsListening())
	{
		m_freed->StopListening();
	}
}


void* TrackerDevice::GetComponent(const char* pchComponentNameAndVersion)
{
	return nullptr;
}


void TrackerDevice::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
	if (unResponseBufferSize >= 1) 
	{
		pchResponseBuffer[0] = 0;
	}
}


void TrackerDevice::EnterStandby() { }