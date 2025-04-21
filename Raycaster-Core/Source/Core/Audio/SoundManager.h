#pragma once

#include "Sound.h"

#include <vector>
#include <unordered_map>

namespace Core::Audio {
    class SoundManager {
    public:
        SoundManager() = default;
        ~SoundManager();
        
        SoundManager(const SoundManager& other) = delete;
        SoundManager(SoundManager&& other) noexcept = delete;
        SoundManager& operator = (const SoundManager& other) = delete;
        SoundManager& operator = (const SoundManager&& other) = delete;

        void Init(Bus* dafaultBus = nullptr, uint32_t initialCapacity = 0);
        void Shutdown();

        void RegisterSound(std::string_view filePath, Sound::Flags flags, Bus* parent = nullptr);
        void RegisterSound(std::string_view name, std::string_view filePath, Sound::Flags flags, Bus* parent = nullptr);
        // Different argument order to prevent unwanted implicit conversions
        void RegisterSound(std::string_view name, Sound::Flags  flags, std::filesystem::path filePath, Bus* parent = nullptr);
        
        void CopySound(std::string_view copyName, Index originalIndex);
        void CopySound(std::string_view copyName, std::string_view originalName);

        void UnregisterSound(std::string_view name);

        void Compact(); // Calling compact invalidates indices of any unregistered sounds
        
        void ReinitSounds();

        bool IndexIsValid(Index index);
        Index ValidateIndex(Index index, std::string_view name);
        Index GetSoundIndex(std::string_view name);

        Sound* GetSound(Index index);
        Sound* GetSound(std::string_view name);

        void SetDafaultBus(Bus* bus);

        size_t SoundCount();
    private:
        uint32_t m_Epoch = 0;
        bool m_IsDense = true;

        std::vector<std::optional<Sound>> m_Sounds;
        std::vector<const char*> m_SoundNames;
        std::unordered_map<uint32_t, std::filesystem::path> m_StreamedFiles;
        std::unordered_map<std::string_view, Index> m_SoundIndices;

        Bus* m_DefaultBus = nullptr;

        std::string_view StoreName(std::string_view name);
        std::string_view StoreName(std::string_view name, uint32_t index);

        std::filesystem::path ConvertFilePath(std::string_view filePath);
        void StoreFilePath(const std::filesystem::path& filePath, uint32_t soundIndex, Sound::Flags flags);
    };
}