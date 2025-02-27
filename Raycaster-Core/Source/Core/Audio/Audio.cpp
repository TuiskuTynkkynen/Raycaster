#include "Audio.h"

#include "Internal.h"

#include "Core/Debug/Debug.h"

namespace Core {
	void Audio::Init() {
		RC_ASSERT(!Internal::System, "Audio System has already been initialized");
		Internal::System = std::make_unique<Internal::AudioSystem>();

		if (ma_engine_init(NULL, &Internal::System->Engine) != MA_SUCCESS) {
			RC_ASSERT("Failed to initialize Audio System");
		}
	}
	
	void Audio::Shutdown() {
		RC_ASSERT(Internal::System, "Audio System has not been initialized");
		Internal::System.reset();

		ma_engine_uninit(&Internal::System->Engine);
	}

	void Audio::Play(const char* filePath) {
		RC_ASSERT(Internal::System, "Audio System has not been initialized");

		ma_result result = ma_engine_play_sound(&Internal::System->Engine, filePath, NULL);
	}
}