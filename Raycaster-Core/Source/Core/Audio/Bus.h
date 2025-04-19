#pragma once

#include "Types.h"

#include <chrono>
#include <variant>
#include <memory>
#include <vector>


namespace Core::Audio {
    class Bus {
        friend class Sound;
        friend class BusManager;
    public:
        Bus();
        Bus(Bus& parent);

        ~Bus();

        Bus(const Bus&) = delete;
        Bus(Bus&& other) noexcept;

        Bus& operator = (const Bus& other) = delete;
        Bus& operator = (const Bus&& other) = delete;

        void Reinit();

        bool IsEnabled();
        // Enables/Disables processing on all sounds attached to this bus and its children
        void SetEnabled(bool enabled);
        // Enables/Disables processing on all sounds attached to this bus and its children after delay milliseconds
        void SetEnabled(bool enabled, std::chrono::milliseconds delay);

        float GetVolume();
        void SetVolume(float volume);
        float GetGaindB();
        void SetGain(float gaindB);

        float GetPitch();
        void SetPitch(float pitch);

        struct Balance {
            float Value;
        };
        struct Pan {
            float Value;
        };
        std::variant<Balance, Pan> GetBalanceOrPan();

        void SetBalance(float balance);
        void SetPan(float pan);

        float GetFadeVolume();
        void SetFadeIn(std::chrono::milliseconds length);
        void SetFadeOut(std::chrono::milliseconds length);
        void SetFade(std::chrono::milliseconds length, float startVolume, float endVolume);

        // Detach old parent and attach new parent
        void AttachParentBus(Bus& parent);
    private:
        std::unique_ptr<Internal::BusObject> m_InternalBus;

        void SwitchParent(Bus* parent);

        Bus* m_Parent;

        void AttachChild(ChildNode child);
        void DetachChild(ChildNode child);

        std::vector<ChildNode> m_Children;

        struct FadeSettings {
            uint64_t StartTime = 0;
            uint64_t Length = 0;
            float StartVolume = 1.0f;
            float EndVolume = 1.0f;
        } m_FadeSettings;
    };
}