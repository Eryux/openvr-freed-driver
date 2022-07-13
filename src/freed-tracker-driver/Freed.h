#pragma once

#include <string>
#include <cstdint>
#include <mutex>
#include <thread>
#include <WinSock2.h>

#include "glm.hpp"

namespace freed_tracker_driver {
	struct FreedData_t {
		glm::vec3 position;
		glm::vec3 rotation;
	};

	class Freed 
	{
		public:
			Freed();
			~Freed();

			FreedData_t GetData();

			bool StartListening(const char* addr, unsigned short port);
			void StopListening();
			bool IsListening();

			void SetAngleShift(float value);
			void SetPositionShift(float value);
			void SetDeviceId(int value);

		private:
			FreedData_t m_lastRecvData;

			std::thread m_worker;
			std::mutex m_lock;
			SOCKET m_udpSocket;
			bool m_listening = false;

			float m_angleShift = 32768.0f;
			float m_positionShift = 64.0f;

			int m_deviceId = 1;

			void Listen();

			float ReadValue(unsigned char high, unsigned char mid, unsigned char low, double shift);
	};
}