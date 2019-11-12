#pragma once
#include "pin.H"

#define LOGPATH "C:\\pin35\\"
#define LOGNAME "guards.log"
#define LOG_BUILD 1

#define LOG_AR(fmt, ...) \
	do { \
		if (!LOG_BUILD) break; \
		SokLogging::logMain(fmt"\n", __VA_ARGS__); \
	} while (0)


class SokLogging {

public:
	static FILE* mainLog;

	static VOID Init();
	static VOID Shutdown();
	static VOID logMain(const char * fmt, ...);

};