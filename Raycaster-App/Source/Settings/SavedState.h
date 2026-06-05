#pragma once

#include "Core/Serialization/Archive.h"

namespace Settings {
    template <typename T>
    class SavedState {
    public:
        SavedState(T state) 
            : m_Current(state), m_Saved(state), m_Default(state) {}

        bool Update(T state) {
            m_Current = state;
            return Saved();
        }

        bool Default() { return m_Current == m_Default; }
        void Reset() { m_Current = m_Default; }

        bool Saved() { return m_Current == m_Saved; }
        void SetSavedExternal() { m_Saved = m_Current; }

        operator T() const { return m_Current; }
        T GetValue() { return m_Current; }

        bool Serialize(Core::Serialization::Archive& archive);
        bool Deserialize(Core::Serialization::Archive& archive);
    private:
        T m_Current;
        T m_Saved;
        const T m_Default;
    };

    template <typename T>
    inline bool SavedState<T>::Serialize(Core::Serialization::Archive& archive) {
        bool result = archive.Write(m_Current);
        if (result) { m_Saved = m_Current; }
        return result;
    }

    template <typename T>
    inline bool SavedState<T>::Deserialize(Core::Serialization::Archive& archive) {
        auto result = archive.Read<T>();
        if (!result.has_value()) { return false; }
        
        m_Current = m_Saved = result.value(); 
        return true;
    }
}