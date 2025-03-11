#include "SoundManager.h"

#include "Core/Debug/Debug.h"

namespace Core::Audio {
    SoundManager::~SoundManager() {
        for (size_t i = 0; i < m_SoundNames.size(); i++) {
            delete[] m_SoundNames[i];
        }
    }

    void SoundManager::RegisterSound(std::string_view filePath, Sound::Flags flags) {
        if (m_SoundIndices.contains(filePath)) {
            return;
        }

        std::string_view name = StoreName(filePath);

        if (m_IsDense) {
            uint32_t index = m_Sounds.size();
            m_SoundIndices.emplace(filePath, Index{ .Epoch = m_Epoch, .Value = index });

            m_Sounds.emplace_back(Sound(filePath, flags));
            return;
        }

        auto iter = std::find_if(m_Sounds.begin(), m_Sounds.end(), [](const std::optional<Sound>& item) { return !item; });
        if (iter == m_Sounds.end()) {
            m_IsDense = true;

            uint32_t index = m_Sounds.size();
            m_SoundIndices.emplace(filePath, Index{ .Epoch = m_Epoch, .Value = index });

            m_Sounds.emplace_back(Sound(filePath, flags));
            return;
        }

        iter->emplace(filePath, flags);

        uint32_t index = iter - m_Sounds.begin();
        m_SoundIndices.emplace(filePath, Index{ .Epoch = m_Epoch, .Value = index });

        m_IsDense = std::find_if(++iter, m_Sounds.end(), [](const std::optional<Sound>& item) { return !item; }) == m_Sounds.end();
    }

    void SoundManager::RegisterSound(std::string_view name, std::string_view filePath, Sound::Flags flags) {
        if (m_SoundIndices.contains(name)) {
            return;
        }

        name = StoreName(name);

        if (m_IsDense) {
            uint32_t index = m_Sounds.size();
            m_SoundIndices.emplace(name, Index{ .Epoch = m_Epoch, .Value = index });

           m_Sounds.emplace_back(Sound(filePath, flags));
            return;
        }

        auto iter = std::find_if(m_Sounds.begin(), m_Sounds.end(), [](const std::optional<Sound>& item) { return !item; });
        if (iter == m_Sounds.end()) {
            m_IsDense = true;

            uint32_t index = m_Sounds.size();
            m_SoundIndices.emplace(name, Index{ .Epoch = m_Epoch, .Value = index });

           m_Sounds.emplace_back(Sound(filePath, flags));
            return;
        }

        iter->emplace(filePath, flags);

        uint32_t index = iter - m_Sounds.begin();
        m_SoundIndices.emplace(name, Index{ .Epoch = m_Epoch, .Value = index });

        m_IsDense = std::find_if(++iter, m_Sounds.end(), [](const std::optional<Sound>& item) { return !item; }) == m_Sounds.end();
    }

    void SoundManager::RegisterSound(std::string_view name, Sound::Flags flags, std::filesystem::path filePath) {
        if (m_SoundIndices.contains(name)) {
            return;
        }

        name = StoreName(name);

        if (m_IsDense) {
            uint32_t index = m_Sounds.size();
            m_SoundIndices.emplace(name, Index{ .Epoch = m_Epoch, .Value = index });

           m_Sounds.emplace_back(Sound(filePath, flags));
            return;
        }

        auto iter = std::find_if(m_Sounds.begin(), m_Sounds.end(), [](const std::optional<Sound>& item) { return !item; });
        if (iter == m_Sounds.end()) {
            m_IsDense = true;

            uint32_t index = m_Sounds.size();
            m_SoundIndices.emplace(name, Index{ .Epoch = m_Epoch, .Value = index });

           m_Sounds.emplace_back(Sound(filePath, flags));
            return;
        }

        iter->emplace(filePath, flags);

        uint32_t index = iter - m_Sounds.begin();
        m_SoundIndices.emplace(name, Index{ .Epoch = m_Epoch, .Value = index });

        m_IsDense = std::find_if(++iter, m_Sounds.end(), [](const std::optional<Sound>& item) { return !item; }) == m_Sounds.end();
    }

    void SoundManager::CopySound(std::string_view copyName, Index originalIndex) {
        if (m_SoundIndices.contains(copyName)) {
            return;
        }

        Sound* original = GetSound(originalIndex);

        if (!original) {
            return;
        }

        if (m_IsDense) {
            uint32_t index = m_Sounds.size();
            m_Sounds.emplace_back(original->Copy());
            
            // Check if copy succeeded
            if (!m_Sounds.back()) {
                m_Sounds.pop_back();
                return;
            }

            m_SoundIndices.emplace(StoreName(copyName), Index{ .Epoch = m_Epoch, .Value = index });
            return;
        }

        auto iter = std::find_if(m_Sounds.begin(), m_Sounds.end(), [](const std::optional<Sound>& item) { return !item; });
        if (iter == m_Sounds.end()) {
            m_IsDense = true;

            // Check if copy succeeded
            if (!m_Sounds.back()) {
                m_Sounds.pop_back();
                return;
            }

            uint32_t index = m_Sounds.size();
            m_SoundIndices.emplace(StoreName(copyName), Index{ .Epoch = m_Epoch, .Value = index });
            
            return;
        }

        iter->emplace(std::move(original->Copy().value()));
        
        // Check if copy succeeded
        if (!iter->has_value()) {
            return;
        }

        uint32_t index = iter - m_Sounds.begin();
        m_SoundIndices.emplace(StoreName(copyName), Index{ .Epoch = m_Epoch, .Value = index });

        m_IsDense = std::find_if(++iter, m_Sounds.end(), [](const std::optional<Sound>& item) { return !item; }) == m_Sounds.end();
    }
    
    void SoundManager::CopySound(std::string_view copyName, std::string_view originalName) {
        auto index = GetSoundIndex(originalName);
        if (!index) {
            return;
        }

        CopySound(copyName, index.value());
    }

    void SoundManager::UnregisterSound(std::string_view name) {
        std::optional<Index> index = GetSoundIndex(name);
        
        m_SoundIndices.erase(name);

        if (index && IndexIsValid(index.value())) {
            m_IsDense = false;

            m_Sounds[index.value().Value].reset();

            delete[] m_SoundNames[index.value().Value];
            m_SoundNames[index.value().Value] = nullptr;
        }
    }
    
    void SoundManager::Compact() {
        if (m_IsDense) {
            return;
        }
        
        RC_ASSERT(m_Sounds.size() == m_SoundNames.size(), "Size of sound and sound names vectors should always be equal");
        bool moved = false;
        for (size_t i = 0; i < m_Sounds.size(); i++) {
            if (!m_Sounds[i]) {
                moved = true;

                m_Sounds[i].emplace(std::move(m_Sounds.back().value()));
                m_Sounds.pop_back();

                m_SoundNames[i] = m_SoundNames.back();
                m_SoundNames.pop_back();
            }
        }

        m_IsDense = true;
        
        if (!moved) {
            return;
        }

        m_Epoch++;
        for (auto& [name, index] : m_SoundIndices) {
            index.Epoch = m_Epoch;
        }
    }

    void SoundManager::ReinitSounds() {
        for (auto& sound : m_Sounds) {
            if (sound) {
                sound.value().Reinit();
            }
        }
    }

    bool SoundManager::IndexIsValid(Index index) {
        return index.Epoch == m_Epoch && index.Value < m_Sounds.size();
    }

    std::optional<SoundManager::Index> SoundManager::GetSoundIndex(std::string_view name) {
        auto iter = m_SoundIndices.find(name);
        
        if (iter == m_SoundIndices.end()) {
            return  std::nullopt;
        }

        return iter->second;
    }

    Sound* SoundManager::GetSound(SoundManager::Index index) {
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
        
        return GetSound(index.value());
    }

    size_t SoundManager::SoundCount() {
        return m_SoundIndices.size();
    }

    std::string_view SoundManager::StoreName(std::string_view name) {
        char* cString = new char[name.size()];
        name.copy(cString, name.size(), (size_t)0);

        m_SoundNames.emplace_back(cString);

        return std::string_view(cString, name.size());
    }
}