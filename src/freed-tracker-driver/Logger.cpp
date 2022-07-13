#include "pch.h"
#include "Logger.h"

#include <stdio.h>
#include <stdarg.h>

static vr::IVRDriverLog* s_pLogFile = nullptr;

#if !defined( WIN32)
#define vsnprintf_s vsnprintf
#endif

bool InitLog(vr::IVRDriverLog* pDriverLog)
{
	if (s_pLogFile) {
		return false;
	}
	s_pLogFile = pDriverLog;
	return s_pLogFile != NULL;
}

void CleanupLog()
{
	s_pLogFile = NULL;
}

static void LogVarArgs(const char* pMsgFormat, va_list args)
{
	char buf[1024];
	vsnprintf_s(buf, sizeof(buf), pMsgFormat, args);

	if (s_pLogFile)
		s_pLogFile->Log(buf);
}


void Log(const char* pMsgFormat, ...)
{
	va_list args;
	va_start(args, pMsgFormat);
	LogVarArgs(pMsgFormat, args);
	va_end(args);
}