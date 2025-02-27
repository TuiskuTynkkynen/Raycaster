#pragma once

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

#include <memory>

namespace Core::Audio::Internal {
	struct AudioSystem {
		ma_engine Engine;
	};

	inline std::unique_ptr<AudioSystem> System;
}