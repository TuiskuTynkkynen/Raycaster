#pragma once

#include "Types.h"

#include <glm/glm.hpp>

#include <string_view>
#include <filesystem>

#define Bit(x) (1 << x)

namespace Core::Audio {
    class Sound {
    public:
        enum FlagEnum : uint8_t {
            None					= 0,
            DisablePitch			= Bit(0),
            DisableSpatialization	= Bit(1),
            StreamData				= Bit(2),
            LoadSynchronous			= Bit(3),
            DecodeDynamically		= Bit(4),

            InvalidFlag = Bit(5), // MUST NOT BE SET
        };
        
        class Flags;

        enum class Positioning {
            Absolute = 0,
            Relative // Relative to listner
        };

        enum class AttenuationMode {
            None = 0,
            Inverse,
            Linear,
            Exponential
        };
    public:
        Sound(const char* filePath, Flags flags);
        Sound(const std::string_view& filePath, Flags flags);
        Sound(const std::filesystem::path& filePath, Flags flags);

        ~Sound();

        Sound(const Sound& other) = delete;
        Sound(Sound&& other) noexcept : m_InternalSound(std::exchange(other.m_InternalSound, nullptr)), m_Flags(std::exchange(other.m_Flags, None)), m_ScheduledFade(other.m_ScheduledFade) {}

        Sound& operator = (const Sound& other) = delete;
        Sound& operator = (const Sound&& other) = delete;

        bool CanReinit();
        void Reinit();

        void ReinitFromFile(const char* filePath); // Used for sound that can not reinit normally
        void ReinitFromFile(const std::string_view& filePath); // Used for sound that can not reinit normally
        void ReinitFromFile(const std::filesystem::path& filePath); // Used for sound that can not reinit normally

        std::optional<Sound> Copy() const; //Copies only the sound and flags
        std::optional<Sound> CopyDeep() const; //Copies sound and it's parameters

        bool IsLooping();
        void SetLooping(bool loops);

        bool IsPlaying();
        bool IsAtEnd();

        std::chrono::milliseconds GetTime();
        std::optional<std::chrono::milliseconds> GetLength();

        void Start(std::chrono::milliseconds fadeLength = std::chrono::milliseconds::zero(), float volumeMin = 0.0f, float volumeMax = 1.0f);
        void Stop();
        
        void Skip(std::chrono::milliseconds duration);
        void SkipTo(std::chrono::milliseconds timepoint);

        void SetVolume(float volume);
        void SetVolumedB(float dBVolume);

        void SetPitch(float pitch);
        void SetBalance(float balance);
        void SetPan(float pan);

        float GetFadeVolume();
        void SetFadeIn(std::chrono::milliseconds length, float endVolume = 1.0f, std::chrono::milliseconds startAfter = std::chrono::milliseconds::zero());
        void SetFadeOut(std::chrono::milliseconds length, std::chrono::milliseconds startAfter = std::chrono::milliseconds::zero());
        void SetFade(std::chrono::milliseconds length, float startVolume, float endVolume, std::chrono::milliseconds startAfter = std::chrono::milliseconds::zero());
        
        void SetSpatialization(bool spatial);
        void SetPositioning(Positioning positioning);
        void SetAttenuation(AttenuationMode attenuation);

        glm::vec3 GetDirectionToListner();

        void SetSpatialData(glm::vec3 position, glm::vec3 direction, glm::vec3 velocity);
        void SetPosition(glm::vec3 position);
        void SetDirection(glm::vec3 direction);
        void SetVelocity(glm::vec3 velocity);

        void SetSpatialParameters(float rolloff, float gainMin, float gainMax, float distanceMin, float distanceMax, float coneInnerAngleRadians, float coneOuterAngleRadians, float coneOuterGain);
        void SetRolloff(float rolloff);
        void SetGainRange(float min, float max);
        void SetDistanceRange(float min, float max);
        void SetCone(float innerAngleRadians, float outerAngleRadians, float outerGain);
        void SetConeDegrees(float innerAngleDegrees, float outerAngleDegrees, float outerGain);

        void SetSpatialFactors(float dopplerFactor, float directionalAttenuationFactor);
        void SetDopplerFactor(float dopplerFactor);
        void SetDirectionalAttenuationFactor(float directionalAttenuationFactor);
    public:
            class Flags {
            public:
                Flags(std::same_as<FlagEnum> auto... flags) { Data = (None + ... + flags); }

                Flags operator~() const { return ~Data; }

                Flags operator&(const Flags& other) const { return Data & other.Data; }

                Flags operator&(const FlagEnum& other) const { return Data & other; }

                Flags operator|(const Flags& other) const { return Data | other.Data; }

                Flags operator|(const FlagEnum& other) const { return Data | other; }

                Flags operator^(const Flags& other) const { return Data ^ other.Data; }

                Flags operator^(const FlagEnum& other) const { return (Data & other) ^ other; }

                operator bool() const { return Data; }
                bool operator!() const { return !Data; }

                uint8_t Data;
            private:
                Flags(uint8_t flags);
            };
    private:
        Sound(Internal::SoundObject* internalSound, Flags flags);
        
        void ReinitInternalSound(Internal::SoundObject* internalSound);

        Internal::SoundObject* m_InternalSound;
        Flags m_Flags;
        
        bool m_ScheduledFade = false;
    };
}