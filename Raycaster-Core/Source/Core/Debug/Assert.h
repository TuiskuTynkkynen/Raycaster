#pragma once

#include "Debug.h"

#include <source_location>

#ifdef ASSERT_ENABLE
	#define RC_INTERNAL_STRINGIFY(x) #x


	#define RC_INTERNAL_ASSERT_WITH_MSG(check, ...) { if(!(check)) { RC_FATAL("Assertion failed: {}", __VA_ARGS__); RC_DEBUGBREAK(); } }
	#define RC_INTERNAL_ASSERT_NO_MSG(check) { if(!(check)) { RC_FATAL("Assertion '{}' failed at {}", RC_INTERNAL_STRINGIFY(check), std::source_location::current()); RC_DEBUGBREAK(); } }

	#define RC_INTERNAL_ASSERT_GET_MACRO(arg1, arg2, macro, ...) macro

	#define RC_ASSERT(...) RC_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__, RC_INTERNAL_ASSERT_WITH_MSG, RC_INTERNAL_ASSERT_NO_MSG)(__VA_ARGS__)
#else
	#define RC_ASSERT(...)
#endif
