#pragma once

#include <openvr_driver.h>
#include "glm.hpp"
#include "gtc/quaternion.hpp"
#include "Freed.h"

namespace freed_tracker_driver {
	class TrackerDevice : public vr::ITrackedDeviceServerDriver
	{
		public:
			TrackerDevice(std::string serialNumber, std::string modelNumber);
			~TrackerDevice();

			void Update();
			virtual vr::DriverPose_t GetPose() override;
			virtual vr::EVRInitError Activate(uint32_t unObjectId) override;
			virtual void Deactivate() override;
			virtual void* GetComponent(const char* pchComponentNameAndVersion) override;
			virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
			virtual void EnterStandby() override;

			std::string GetSerialNumber();

			static inline vr::DriverPose_t DefaultPose(bool connected = true, bool tracking = true)
			{
				vr::DriverPose_t pose { 0 };
				pose.deviceIsConnected = connected;
				pose.poseIsValid = tracking;
				pose.result = tracking ? vr::ETrackingResult::TrackingResult_Running_OK : vr::ETrackingResult::TrackingResult_Running_OutOfRange;
				pose.willDriftInYaw = false;
				pose.shouldApplyHeadModel = false;
				pose.qDriverFromHeadRotation.w = pose.qWorldFromDriverRotation.w = pose.qRotation.w = 1.0;
				return pose;
			}

		private:
			vr::TrackedDeviceIndex_t m_deviceIndex = vr::k_unTrackedDeviceIndexInvalid;
			vr::PropertyContainerHandle_t m_propertyContainer;

			std::string m_serialNumber;
			std::string m_modelNumber;

			bool m_initialized = false;

			Freed* m_freed;

			glm::vec3 m_position_offset;
			glm::quat m_rotation_offset;

			glm::vec3 m_universe_position;
			float m_universe_yaw;
	};
}