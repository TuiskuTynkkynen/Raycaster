#pragma once

#include "SoundManager.h"

#include <vector>
#include <string_view>

namespace Core::Audio {
    void Init();
    void Shutdown();

    // Load, play and clean up a sound without registering in the SoundManager
    void PlayInlineSound(std::string_view filePath);
    // Load, play and clean up a sound without registering in the SoundManager
    void PlayInlineSound(std::filesystem::path& filePath);

    void Play(SoundManager::Index soundIndex);
    void Play(std::string_view soundName);
    // Validate index and play sound
    void Play(SoundManager::Index& soundIndex, std::string_view soundName);
    
    void Pause(SoundManager::Index soundIndex);
    void Pause(std::string_view soundName);
    // Validate index and pause sound
    void Pause(SoundManager::Index& soundIndex, std::string_view soundName);

    void Stop(SoundManager::Index soundIndex);
    void Stop(std::string_view soundName);
    // Validate index and stop sound
    void Stop(SoundManager::Index& soundIndex, std::string_view soundName);
    
    SoundManager& GetSoundManager();
    
    bool ValidateSoundIndex(SoundManager::Index& index, std::string_view name);
    SoundManager::Index GetSoundIndex(std::string_view name);

    Sound* GetSound(SoundManager::Index index);
    Sound* GetSound(std::string_view name);

    std::vector<std::string_view> GetDevices();

    void SetDevice(size_t deviceIndex);
    void SetDevice(std::string_view name);
    // Switch to default device
    void SetDevice(); 
}

