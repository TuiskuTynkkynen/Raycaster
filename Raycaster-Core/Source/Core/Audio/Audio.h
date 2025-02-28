#pragma once

#include <vector>
#include <string_view>

namespace Core::Audio {
	void Init();
	void Shutdown();

	void Play(const char* filePath);
	
	std::vector<std::string_view> GetDevices();

	void SetDevice(size_t deviceIndex);
	void SetDevice(std::string_view deviceIndex);
}

