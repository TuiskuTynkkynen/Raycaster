#pragma once

#include <glm/glm.hpp>

#include <string_view>
#include <filesystem>

#define Bit(x) (1 << x)

struct InternalSoundObject;

namespace Core::Audio {
    namespace SoundTypes {
        enum Flags : uint8_t {
            None					= 0,
            DisablePitch			= Bit(0),
            DisableSpatialization	= Bit(1),
            StreamData				= Bit(2),
            LoadSynchronous			= Bit(3),
            DecodeDynamically		= Bit(4),
        };

        enum class SoundPositioning {
            Absolute = 0,
            Relative // Relative to listner
        };

        enum class SoundAttenuationMode {
            None = 0,
            Inverse,
            Linear,
            Exponential
        };
    }

    class Sound {
    public:
        Sound(const char* filePath, uint8_t flags);
        Sound(const std::string_view& filePath, uint8_t flags);
        Sound(const std::filesystem::path& filePath, uint8_t flags);

        ~Sound();

        Sound(const Sound& other) = delete;
        Sound(Sound&& other) noexcept : m_InternalSound(std::exchange(other.m_InternalSound, nullptr)), m_Flags(std::exchange(other.m_Flags, SoundTypes::None)) {}

        Sound& operator = (const Sound& other) = delete;
        Sound& operator = (const Sound&& other) = delete;

        void Reinit();

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
        void SetPositioning(SoundTypes::SoundPositioning positioning);
        void SetAttenuation(SoundTypes::SoundAttenuationMode attenuation);

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
    private:
        Sound(InternalSoundObject* internalSound, uint8_t flags);

        InternalSoundObject* m_InternalSound;
        uint8_t m_Flags;
    };
}