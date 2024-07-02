#pragma once

//Debug mode
#if defined(DEBUG) | defined(RELEASE)
	#define RC_DEBUG_MODE
	#ifdef LOG_ENABLE
		#define ASSERT_ENABLE
	#endif
#endif

//Platform
#ifdef _WIN32
	#ifdef _WIN64
		#define PLATFORM_WINDOWS
	#else
		#error "Windows x86 Builds not supported"
	#endif
#elif defined(__ANDROID__)
	#error "Android not supported"
#elif defined(__linux__)
	#define PLATFORM_LINUX
#else
	#error "Unknown platform"
#endif

//Debug breaks
#ifdef RC_DEBUG_MODE
	#if defined(PLATFORM_WINDOWS)
		#define RC_DEBUGBREAK() __debugbreak()
	#elif defined(PLATFORM_LINUX)
		#include <signal.h>
		#define RC_DEBUGBREAK() raise(SIGTRAP)
	#endif
#else
	#define RC_DEBUGBREAK()
#endif

#include "Log.h"
#include "Assert.h"