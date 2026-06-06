#pragma once

#include "Core/Serialization/Archive.h"

namespace Settings {
    template <typename T>
    struct NoCallback {
        inline void operator()(T t){}
    };

    template <typename T, typename UpdateCallback = NoCallback<T>>
    class SavedState {
    public:
        SavedState(T state, UpdateCallback callback = {})
            : m_Current(state), m_Saved(state), m_Default(state), m_Callback(callback){}

        bool Update(T state) {
            if (m_Current != state) { m_Callback(state); }
            m_Current = state;
            return Saved();
        }

        bool Default() { return m_Current == m_Default; }
        void Reset() { Update(m_Default); }

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
        [[no_unique_address]] UpdateCallback m_Callback;
    };

    template <typename T, typename C>
    inline bool SavedState<T, C>::Serialize(Core::Serialization::Archive& archive) {
        bool result = archive.Write(m_Current);
        if (result) { SetSavedExternal(); }
        return result;
    }

    template <typename T, typename C>
    inline bool SavedState<T, C>::Deserialize(Core::Serialization::Archive& archive) {
        auto result = archive.Read<T>();
        if (!result.has_value()) { return false; }
        
        Update(result.value());
        SetSavedExternal();
        return true;
    }
}