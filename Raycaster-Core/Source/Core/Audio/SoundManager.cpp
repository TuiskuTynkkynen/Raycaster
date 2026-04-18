#include "SoundManager.h"

#include "Core/Debug/Debug.h"

#include <algorithm>

namespace Core::Audio {
    SoundManager::~SoundManager() {
        for (size_t i = 0; i < m_SoundNames.size(); i++) {
            delete[] m_SoundNames[i];
        }
    }

    void SoundManager::Init(Bus* dafaultBus, uint32_t initialCapacity) { 
        RC_ASSERT(m_Sounds.empty() && m_SoundIndices.empty() && m_SoundNames.empty() && m_StreamedFiles.empty(), "SoundManager has already been initialized");
               
        m_Sounds.reserve(initialCapacity); 
        m_SoundIndices.reserve(initialCapacity); 
        m_SoundNames.reserve(initialCapacity);
        
        m_Epoch = 0;
        m_IsDense = true;
        m_DefaultBus = dafaultBus;
    }

    void SoundManager::Shutdown() {
        for (size_t i = 0; i < m_SoundNames.size(); i++) {
            delete[] m_SoundNames[i];
        }

        m_Sounds.clear();
        m_Sounds.shrink_to_fit();

        m_SoundNames.clear();
        m_SoundNames.shrink_to_fit();
        
        m_StreamedFiles.clear();
        m_StreamedFiles.rehash(0);
        
        m_SoundIndices.clear();
        m_SoundIndices.rehash(0);
        
        m_Epoch = 0;
        m_IsDense = true;
        m_DefaultBus = nullptr;
    }

    Sound& SoundManager::RegisterSound(std::string_view filePath, Sound::Flags flags, Bus* parent) {
        return RegisterSound(filePath, flags, ConvertFilePath(filePath), parent);
    }

    Sound& SoundManager::RegisterSound(std::string_view name, std::string_view filePath, Sound::Flags flags, Bus* parent) {
        return RegisterSound(name, flags, ConvertFilePath(filePath), parent);
    }

    Sound& SoundManager::RegisterSound(std::string_view name, Sound::Flags flags, std::filesystem::path filePath, Bus* parent) {
        if (auto sound = GetSound(name); sound != nullptr) {
            RC_INFO("Tried to register sound with name, {}, but one already exists", name);
            return *sound;
        }

        auto iter = NextSlot();
        m_IsDense |= std::ranges::all_of(iter + 1, m_Sounds.end(), &std::optional<Sound>::has_value);

        RC_ASSERT(iter - m_Sounds.begin() <= std::numeric_limits<uint32_t>::max(), "Audio System supports only up to UINT32_MAX concurrent registered sounds");
        uint32_t index = static_cast<int32_t>(iter - m_Sounds.begin());
        m_SoundIndices.emplace(StoreName(name, index), Index{ .Epoch = m_Epoch, .Value = index });
        StoreFilePath(filePath, index, flags);

        parent = parent ? parent : m_DefaultBus;
        return iter->emplace(filePath, flags, parent);
    }

    Sound& SoundManager::RegisterSound(std::string_view name, std::span<const std::byte> embededAudio, Sound::Flags flags, Bus* parent) {
        if (auto sound = GetSound(name); sound != nullptr) {
            RC_INFO("Tried to register sound with name, {}, but one already exists", name);
            return *sound;
        }

        auto iter = NextSlot();
        m_IsDense |= std::ranges::all_of(iter + 1, m_Sounds.end(), &std::optional<Sound>::has_value);
        
        RC_ASSERT(iter - m_Sounds.begin() <= std::numeric_limits<uint32_t>::max(), "Audio System supports only up to UINT32_MAX concurrent registered sounds");
        uint32_t index = static_cast<int32_t>(iter - m_Sounds.begin());
        m_SoundIndices.emplace(StoreName(name, index), Index{ .Epoch = m_Epoch, .Value = index });
        
        parent = parent ? parent : m_DefaultBus;
        return iter->emplace(embededAudio, flags, parent);
    }

    Sound* SoundManager::CopySound(std::string_view copyName, Index originalIndex) {
        if (auto sound = GetSound(copyName); sound != nullptr) {
            RC_INFO("Tried to create a sound copy with name = {}, but that name already exists", copyName);
            return sound;
        }

        Sound* original = GetSound(originalIndex);
        if (!original) {
            RC_WARN("Tried to create a sound copy with name = {}, but original sound does not exist", copyName);
            return nullptr;
        }

        auto copy = original->Copy();
        if (!copy) {
            RC_WARN("Failed to create a sound copy with name = {}", copyName);
            return nullptr;
        }

        auto iter = NextSlot();
        RC_ASSERT(iter - m_Sounds.begin() <= std::numeric_limits<uint32_t>::max(), "Audio System supports only up to UINT32_MAX concurrent registered sounds");
        uint32_t index = static_cast<int32_t>(iter - m_Sounds.begin());

        m_SoundIndices.emplace(StoreName(copyName, index), Index{ .Epoch = m_Epoch, .Value = index });
        m_IsDense |= std::ranges::all_of(iter + 1, m_Sounds.end(), &std::optional<Sound>::has_value);

        return &iter->emplace(std::move(copy.value()));
    }

    Sound* SoundManager::CopySound(std::string_view copyName, std::string_view originalName) {
        auto index = GetSoundIndex(originalName);
        if (!index) {
            RC_WARN("Tried to create a copy from sound with name = {}, but it does not exist", originalName);
            return nullptr;
        }

        return CopySound(copyName, index);
    }

    void SoundManager::UnregisterSound(std::string_view name) {
        Index index = GetSoundIndex(name);

        m_SoundIndices.erase(name);

        if (IndexIsValid(index)) {
            m_IsDense = index.Value == (m_Sounds.size() - 1); // If last element, m_Sounds is still dense

            m_Sounds[index.Value].reset();

            delete[] m_SoundNames[index.Value];
            m_SoundNames[index.Value] = nullptr;

            auto iter = m_StreamedFiles.find(index.Value);
            if (iter != m_StreamedFiles.end()) {
                m_StreamedFiles.erase(iter);
            }
        }
    }

    void SoundManager::Compact() {
        if (m_IsDense) {
            return;
        }

        RC_ASSERT(m_Sounds.size() == m_SoundNames.size(), "Size of sound and sound names vectors should always be equal");
        struct Moved {
            uint32_t From;
            uint32_t To;
        };
        std::vector<Moved> moved;

        // First pass move all nullopts to back
        for (size_t i = 0; i < m_Sounds.size(); i++) {
            if (!m_Sounds[i]) {
                RC_ASSERT(m_Sounds.size() - 1 <= std::numeric_limits<uint32_t>::max(), "Audio System supports only up to UINT32_MAX concurrent registered sounds");
                uint32_t oldIndex = static_cast<uint32_t>(m_Sounds.size() - 1);

                moved.emplace_back(static_cast<uint32_t>(oldIndex), static_cast<uint32_t>(i)); // oldIndex and i can never be larger than UINT32_MAX

                auto& back = m_Sounds.back();
                if (back) {
                    m_Sounds[i].emplace(std::move(m_Sounds.back().value()));
                }
                m_Sounds.pop_back();

                m_SoundNames[i] = m_SoundNames.back();
                m_SoundNames.pop_back();

                auto iter = m_StreamedFiles.find(oldIndex);
                if (iter != m_StreamedFiles.end()) {
                    m_StreamedFiles.emplace(static_cast<uint32_t>(i), iter->second); // i can never be larger than UINT32_MAX
                }
            }
        }

        // Second pass remove all nullopts from back
        for (size_t i = m_Sounds.size(); i > 0;) {
            if (m_Sounds[--i]) {
                break;
            }

            m_Sounds.pop_back();
            m_SoundNames.pop_back();
        }

        m_IsDense = true;

        if (moved.empty()) {
            return;
        }

        m_Epoch++;
        for (auto& [name, index] : m_SoundIndices) {
            for (size_t i = 0; i < moved.size(); i++) {
                if (moved[i].From == index.Value) {
                    index.Value = moved[i].To;

                    moved[i] = moved.back();
                    moved.pop_back();
                    break;
                }
            }

            index.Epoch = m_Epoch;
        }
    }

    void SoundManager::ReinitSounds() {
        for (size_t i = 0; i < m_Sounds.size(); i++) {
            if (!m_Sounds[i] || !m_Sounds[i].value().CanReinit()) {
                continue;
            }

            m_Sounds[i].value().Reinit();
        }

        for (auto& [index, file] : m_StreamedFiles) {
            if (!m_Sounds[index]) {
                continue;
            }

            m_Sounds[index].value().ReinitFromFile(file);
        }
    }

    bool SoundManager::IndexIsValid(Index index) {
        return index.Epoch == m_Epoch && index.Value < m_Sounds.size();
    }

    Index SoundManager::ValidateIndex(Index index, std::string_view name) {
        // Already invalidated or valid index
        if (!index || IndexIsValid(index)) {
            return index;
        }

        if (auto iter = m_SoundIndices.find(name); iter != m_SoundIndices.end()) {
            return iter->second;
        }

        // Could not validate
        return Index{};
    }

    Index SoundManager::GetSoundIndex(std::string_view name) {
        auto iter = m_SoundIndices.find(name);

        if (iter == m_SoundIndices.end()) {
            return  Index{}; // Return invalidated index
        }

        return iter->second;
    }

    Sound* SoundManager::GetSound(Index index) {
        if (IndexIsValid(index) && m_Sounds[index.Value]) {
            return &m_Sounds[index.Value].value();
        }

        return nullptr;
    }

    Sound* SoundManager::GetSound(std::string_view name) {
        auto index = GetSoundIndex(name);

        if (!index) {
            return nullptr;
        }

        return GetSound(index);
    }

    void SoundManager::SetDafaultBus(Bus* bus) {
        m_DefaultBus = bus;
    }

    size_t SoundManager::SoundCount() {
        return m_SoundIndices.size();
    }

    std::vector<std::optional<Sound>>::iterator SoundManager::NextSlot() {
        std::vector<std::optional<Sound>>::iterator firstEmpty;
        m_IsDense |= (firstEmpty = std::ranges::find(m_Sounds, false, &std::optional<Sound>::has_value)) == m_Sounds.end();

        if (m_IsDense) {
            m_Sounds.emplace_back();
            firstEmpty = std::prev(m_Sounds.end());
        }

        return firstEmpty;
    }

    std::string_view SoundManager::StoreName(std::string_view name, uint32_t index) {
        char* cString = new char[name.size()];
        name.copy(cString, name.size(), (size_t)0);

        RC_ASSERT(index <= m_SoundNames.size());
        if(index == m_SoundNames.size()) {
            m_SoundNames.emplace_back();
        }

        m_SoundNames[index] = cString;
        return std::string_view(cString, name.size());
    }

    std::filesystem::path SoundManager::ConvertFilePath(std::string_view filePath) {
        return ApplicationDirectory().append(filePath);
    }

    void SoundManager::StoreFilePath(const std::filesystem::path& filePath, uint32_t soundIndex, Sound::Flags flags) {
        if (flags ^ Sound::StreamData) {
            return;
        }

        m_StreamedFiles.emplace(soundIndex, filePath);
    }
}