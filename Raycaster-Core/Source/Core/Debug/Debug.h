#pragma once

#include <Platform.h>

//Debug mode
#if defined(DEBUG) | defined(RELEASE)
	#define RC_DEBUG_MODE
	#ifdef LOG_ENABLE
		#define ASSERT_ENABLE
	#endif
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