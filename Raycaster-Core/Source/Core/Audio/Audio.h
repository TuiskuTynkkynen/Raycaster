#pragma once

#include "SoundManager.h"

#include <vector>
#include <string_view>

namespace Core::Audio {
    void Init();
    void Shutdown();

    void SetMasterVolume(float volume);
    // Convert dB to linear volume and set it
    void SetMasterGain(float gaindB);

    void SetWorldUp(glm::vec3 up);
    void SetListnerCone(float innerAngle, float outerAngle, float outerGaindB);
    void SetListnerConeDegrees(float innerAngleDegrees, float outerAngleDegrees, float outerGaindB);

    void SetListnerSpatialData(glm::vec3 position, glm::vec3 direction, glm::vec3 velocity);
    void SetListnerPosition(glm::vec3 position);
    void SetListnerDirection(glm::vec3 direction);
    void SetListnerVelocity(glm::vec3 velocity);

    // Load, play and clean up a sound without registering in the SoundManager
    void PlayInlineSound(std::string_view filePath);
    // Load, play and clean up a sound without registering in the SoundManager
    void PlayInlineSound(std::filesystem::path& filePath);

    void Play(Index soundIndex);
    void Play(std::string_view soundName);
    // Validate index and play sound
    void Play(Index& soundIndex, std::string_view soundName);

    void Pause(Index soundIndex);
    void Pause(std::string_view soundName);
    // Validate index and pause sound
    void Pause(Index& soundIndex, std::string_view soundName);

    void Stop(Index soundIndex);
    void Stop(std::string_view soundName);
    // Validate index and stop sound
    void Stop(Index& soundIndex, std::string_view soundName);

    SoundManager& GetSoundManager();

    bool ValidateSoundIndex(Index& index, std::string_view name);
    Index GetSoundIndex(std::string_view name);

    Sound* GetSound(Index index);
    Sound* GetSound(std::string_view name);

    std::vector<std::string_view> GetDevices();

    void SetDevice(size_t deviceIndex);
    void SetDevice(std::string_view name);
    // Switch to default device
    void SetDevice();
}

