#include "pch.h"
#include "Freed.h"
#include "Logger.h"

#define _FREED_BUFF_LEN 128


using namespace freed_tracker_driver;


Freed::Freed()
{
	m_udpSocket = SOCKET();
	m_lastRecvData = FreedData_t();
}

Freed::~Freed()
{
	StopListening();
}


bool Freed::StartListening(const char* addr, unsigned short port)
{
	if (m_listening) {
		return false;
	}

	m_udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_udpSocket == INVALID_SOCKET) {
		return false;
	}

	char enable = 1;
	setsockopt(m_udpSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

	DWORD timeout = 500;
	setsockopt(m_udpSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));

	sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.S_un.S_addr = inet_addr(addr);
	sockaddr.sin_port = htons(port);

	int r = bind(m_udpSocket, reinterpret_cast<SOCKADDR*>(&sockaddr), sizeof(sockaddr));

	if (r != 0) {
		return false;
	}

	m_worker = std::thread(&Freed::Listen, this);

	return true;
}


void Freed::StopListening()
{
	if (m_listening)
	{
		m_listening = false;

		m_worker.join();

		closesocket(m_udpSocket);
	}

	Log("Stop listening for udp packet");
}


void Freed::Listen()
{
	m_listening = true;

	sockaddr_in from;
	int from_len = sizeof(from);

	Log("Start listening for udp packet");
	
	while (m_listening)
	{
		char buff[_FREED_BUFF_LEN] = { 0 };
		recvfrom(m_udpSocket, buff, _FREED_BUFF_LEN, 0, reinterpret_cast<sockaddr*>(&from), &from_len);

		int cur = 0;
		while (cur + 29 < _FREED_BUFF_LEN)
		{
			unsigned char checksum = buff[cur + 28];

			unsigned char calculate_checksum = 0x40;
			for (int i = cur; i < cur + 28; ++i) {
				calculate_checksum -= buff[cur + i];
			}

			if (checksum == calculate_checksum)
			{
				if (buff[cur] == (char)0xd1 && m_deviceId == buff[cur + 1])
				{
					FreedData_t data;
					data.rotation.x = ReadValue(buff[cur + 2], buff[cur + 3], buff[cur + 4], m_angleShift);
					data.rotation.y = ReadValue(buff[cur + 5], buff[cur + 6], buff[cur + 7], m_angleShift);
					data.rotation.z = ReadValue(buff[cur + 8], buff[cur + 9], buff[cur + 10], m_angleShift);
					data.position.x = ReadValue(buff[cur + 11], buff[cur + 12], buff[cur + 13], m_positionShift) * 0.001f;
					data.position.y = ReadValue(buff[cur + 14], buff[cur + 15], buff[cur + 16], m_positionShift) * 0.001f;
					data.position.z = ReadValue(buff[cur + 17], buff[cur + 18], buff[cur + 19], m_positionShift) * 0.001f;

					m_lock.lock();
					m_lastRecvData = data;
					m_lock.unlock();
				}

				cur += 28;
			}
			else
			{
				cur++;
			}
		}
	}
}


float Freed::ReadValue(unsigned char high, unsigned char mid, unsigned char low, double shift)
{
	int value = (high << 16) | (mid << 8) | low;
	if (value & 0x800000) {
		value |= ~0xffffff;
	}
	return static_cast<float>(value) / shift;
}


bool Freed::IsListening()
{
	return m_listening;
}


FreedData_t Freed::GetData()
{
	return m_lastRecvData;
}


void Freed::SetAngleShift(float value)
{
	m_angleShift = value;
}

void Freed::SetPositionShift(float value)
{
	m_positionShift = value;
}

void Freed::SetDeviceId(int value)
{
	m_deviceId = value;
}