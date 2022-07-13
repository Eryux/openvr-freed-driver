#pragma once

#include <string>
#include <openvr_driver.h>

extern void Log(const char* pchFormat, ...);

extern bool InitLog(vr::IVRDriverLog* pDriverLog);

extern void CleanupLog();