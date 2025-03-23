#include "Sound.h"

#include "Internal.h"
#include "Bus.h"
#include "Core/Debug/Debug.h"

#include "miniaudio/miniaudio.h"

namespace Core::Audio {
    static ma_uint32 ParseFlags(Sound::Flags flags) {
        ma_uint32 result = 0;

        result |= (flags & Sound::DisablePitch) * MA_SOUND_FLAG_NO_PITCH;
        result |= (flags & Sound::DisableSpatialization) * MA_SOUND_FLAG_NO_SPATIALIZATION;
        result |= (flags & Sound::StreamData) * MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_STREAM;
        result |= (flags ^ Sound::LoadSynchronous) * MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC;
        result |= (flags ^ Sound::DecodeDynamically) * MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE;

        return result;
    }

    static Internal::SoundObject* CreateShallowCopy(const Internal::SoundObject* original, Sound::Flags flags, ma_sound_group* group) {
        Internal::SoundObject* copy = new Internal::SoundObject;
        ma_result result = ma_sound_init_copy(Internal::System->Engine, original, flags, group, copy);

        if (result != MA_SUCCESS) {
            RC_WARN("Cypying sound failed with error {}", (int32_t)result);
            delete copy;
            return nullptr;
        }
        
        return copy;
    }
    
    static void CopyInternalAttributes(Internal::SoundObject* copy, const Internal::SoundObject* original, Sound::Flags flags){
        ma_sound_set_looping(copy, ma_sound_is_looping(original));
        ma_sound_set_volume(copy, ma_sound_get_volume(original));

        if (flags ^ Sound::DisablePitch) {
            ma_sound_set_pitch(copy, ma_sound_get_pitch(original));
        }

        ma_sound_set_pan_mode(copy, ma_sound_get_pan_mode(original));
        ma_sound_set_pan(copy, ma_sound_get_pan(original));

        if (flags ^ Sound::DisableSpatialization) {
            ma_sound_set_spatialization_enabled(copy, ma_sound_is_spatialization_enabled(original));
            ma_sound_set_positioning(copy, ma_sound_get_positioning(original));

            {
                ma_vec3f position = ma_sound_get_position(original);
                ma_sound_set_position(copy, position.x, position.y, position.z);

                ma_vec3f direction = ma_sound_get_direction(original);
                ma_sound_set_direction(copy, direction.x, direction.y, direction.z);

                ma_vec3f velocity = ma_sound_get_velocity(original);
                ma_sound_set_velocity(copy, velocity.x, velocity.y, velocity.z);
            }

            ma_sound_set_rolloff(copy, ma_sound_get_rolloff(original));
            ma_sound_set_min_gain(copy, ma_sound_get_min_gain(original));
            ma_sound_set_max_gain(copy, ma_sound_get_max_gain(original));
            ma_sound_set_min_distance(copy, ma_sound_get_min_distance(original));
            ma_sound_set_max_distance(copy, ma_sound_get_max_distance(original));

            {
                float innerAngle, outerAngle, outerGain;
                ma_sound_get_cone(original, &innerAngle, &outerAngle, &outerGain);
                ma_sound_set_cone(copy, innerAngle, outerAngle, outerGain);
            }

            ma_sound_set_doppler_factor(copy, ma_sound_get_doppler_factor(original));
            ma_sound_set_directional_attenuation_factor(copy, ma_sound_get_directional_attenuation_factor(original));
        }
    }

    static Internal::SoundObject* CreateDeepCopy(const Internal::SoundObject* original, Sound::Flags flags, ma_sound_group* group) {
        Internal::SoundObject* copy = CreateShallowCopy(original, flags, group);

        if (!copy) {
            return nullptr;
        }

        CopyInternalAttributes(copy, original, flags);

        return copy;
    }
}

namespace Core::Audio {
    Sound::Sound(const std::filesystem::path& filePath, Flags flags, Bus* parent) : m_Flags(flags) {
        RC_ASSERT(Internal::System, "Tried to create sound before initializing Audio System");
        m_InternalSound = new Internal::SoundObject;
        
        std::string fileString = filePath.string();

        ma_sound_group* group = parent ? parent->m_InternalBus.get() : nullptr;
        ma_result result = ma_sound_init_from_file(Internal::System->Engine, fileString.c_str(), ParseFlags(m_Flags), group, nullptr, m_InternalSound);
        if (result != MA_SUCCESS) {
            RC_WARN("Initializing sound, \"{}\", failed with error {}", filePath.string(), (int32_t)result);
        }

        SwitchParent(parent);
    }

    Sound::Sound(const std::string_view& filePath, Flags flags, Bus* parent) : m_Flags(flags) {
        RC_ASSERT(Internal::System, "Tried to create sound before initializing Audio System");
        m_InternalSound = new Internal::SoundObject;

        std::filesystem::path directoryPath = filePath;
        if (directoryPath.is_relative()) {
            directoryPath = std::filesystem::current_path() / "Source" / "Audio" / directoryPath;
        }
        std::string fileString = directoryPath.string();
        
        ma_sound_group* group = parent ? parent->m_InternalBus.get() : nullptr;
        ma_result result = ma_sound_init_from_file(Internal::System->Engine, fileString.c_str(), ParseFlags(m_Flags), group, nullptr, m_InternalSound);
        if (result != MA_SUCCESS) {
            RC_WARN("Initializing sound, \"{}\", failed with error {}", filePath, (int32_t)result);
        }

        SwitchParent(parent);
    }

    Sound::Sound(const char* filePath, Flags flags, Bus* parent) : m_Flags(flags) {
        RC_ASSERT(Internal::System, "Tried to create sound before initializing Audio System");
        m_InternalSound = new Internal::SoundObject;
        
        std::filesystem::path directoryPath = filePath;
        if (directoryPath.is_relative()) {
            directoryPath = std::filesystem::current_path() / "Source" / "Audio" / directoryPath;
        }
        std::string fileString = directoryPath.string();

        ma_sound_group* group = parent ? parent->m_InternalBus.get() : nullptr;
        ma_result result = ma_sound_init_from_file(Internal::System->Engine, fileString.c_str(), ParseFlags(m_Flags), group, nullptr, m_InternalSound);
        if (result != MA_SUCCESS) {
            RC_WARN("Initializing sound, \"{}\", failed with error {}", filePath, (int32_t)result);
        }

        SwitchParent(parent);
    }

    Sound::Sound(Internal::SoundObject* internalSound, Flags flags, Bus* parent) : m_Flags(flags), m_InternalSound(internalSound) {
        RC_ASSERT(Internal::System, "Tried to create sound before initializing Audio System");
        SwitchParent(parent);
    }

    Sound::~Sound() {
        SwitchParent(nullptr);

        ma_sound_uninit(m_InternalSound);
        delete m_InternalSound;
    }
    
    Sound::Sound(Sound&& other) noexcept : m_InternalSound(std::exchange(other.m_InternalSound, nullptr)), m_Flags(other.m_Flags), m_ScheduledFade(other.m_ScheduledFade), m_FadeSettings(other.m_FadeSettings) {
        SwitchParent(other.m_Parent);
        other.SwitchParent(nullptr);
    }

    bool Sound::CanReinit() {
        return m_Flags ^ StreamData;
    }

    void Sound::Reinit() {
        if (!CanReinit()) {
            RC_WARN("Can not reinitialize sound intialized with StreamData flag. ReinitFromFile should be called instead");
            return;
        }

        ma_sound_group* group = m_Parent ? m_Parent->m_InternalBus.get() : nullptr;
        Internal::SoundObject* internalSoundCopy = CreateDeepCopy(m_InternalSound, m_Flags, group);

        if (!internalSoundCopy) {
            RC_WARN("Reinitializing sound failed. Could not successfully copy internal sound object");
            return;
        }

        ReinitInternalSound(internalSoundCopy);

        // Clean up old ma_sound
        ma_sound_uninit(m_InternalSound);
        delete m_InternalSound;

        m_InternalSound = internalSoundCopy;
    }
    
    void Sound::ReinitFromFile(const char* filePath) {
        std::filesystem::path directoryPath = filePath;
        if (directoryPath.is_relative()) {
            directoryPath = std::filesystem::current_path() / "Source" / "Audio" / directoryPath;
        }

        ReinitFromFile(directoryPath);
    }

    void Sound::ReinitFromFile(const std::string_view& filePath) {
        std::filesystem::path directoryPath = filePath;
        if (directoryPath.is_relative()) {
            directoryPath = std::filesystem::current_path() / "Source" / "Audio" / directoryPath;
        }

        ReinitFromFile(directoryPath);
    }

    void Sound::ReinitFromFile(const std::filesystem::path& filePath) {
        Internal::SoundObject* internalSoundCopy = new Internal::SoundObject;

        std::string fileString = filePath.string();

        ma_sound_group* group = m_Parent ? m_Parent->m_InternalBus.get() : nullptr;
        ma_result result = ma_sound_init_from_file(Internal::System->Engine, fileString.c_str(), ParseFlags(m_Flags), group, nullptr, internalSoundCopy);
        if (result != MA_SUCCESS) {
            RC_WARN("Reinitializing sound from file, \"{}\", failed with error {}", filePath.string(), (int32_t)result);
            
            delete internalSoundCopy;
            return;
        }

        CopyInternalAttributes(internalSoundCopy, m_InternalSound, m_Flags);
        ReinitInternalSound(internalSoundCopy);

        // Clean up old ma_sound
        ma_sound_uninit(m_InternalSound);
        delete m_InternalSound;

        m_InternalSound = internalSoundCopy;
    }

    void Sound::ReinitInternalSound(Internal::SoundObject* internalSound) {
        {
            auto length = GetLength();
            std::chrono::milliseconds time = GetTime();

            if (length) {
                if (time >= length.value()) {
                    time %= length.value();
                }
            }

            using namespace std::chrono_literals;
            ma_uint64 frame = SAMPLERATE * time / 1s;
            ma_sound_seek_to_pcm_frame(internalSound, frame);
        }

        if (m_FadeSettings.StartVolume != 1.0f || m_FadeSettings.EndVolume != 1.0f) {
            RC_ASSERT(Internal::System->Engine);

            ma_uint64 currentTime = ma_engine_get_time_in_pcm_frames(Internal::System->Engine);
            ma_uint64 oldEngineTime = ma_engine_get_time_in_pcm_frames(m_InternalSound->engineNode.pEngine);
            ma_int64 relativeStartTime = m_FadeSettings.StartTime - oldEngineTime;

            if (m_ScheduledFade) {
                if (relativeStartTime < 0 && (ma_uint64)glm::abs(relativeStartTime) > currentTime) { // StartTime would underflow, so update Length and StartVolume instead
                    m_FadeSettings.StartTime = currentTime;
                    m_FadeSettings.Length = currentTime + relativeStartTime;
                    m_FadeSettings.StartVolume = GetFadeVolume();
                } else {
                    m_FadeSettings.StartTime = currentTime + relativeStartTime;
                }

                ma_sound_set_fade_start_in_pcm_frames(internalSound, m_FadeSettings.StartVolume, m_FadeSettings.EndVolume, m_FadeSettings.Length, m_FadeSettings.StartTime);
            } else {
                m_FadeSettings.StartTime = currentTime;

                if (relativeStartTime < 0) { // Fade has started, so use update values
                    m_FadeSettings.Length = currentTime + relativeStartTime;
                    m_FadeSettings.StartVolume = GetFadeVolume();
                }

                ma_sound_set_fade_in_pcm_frames(internalSound, m_FadeSettings.StartVolume, m_FadeSettings.EndVolume, m_FadeSettings.Length);
            }
        }
        
        if (IsPlaying()) {
            Stop();
            ma_sound_start(internalSound);
        }
    }

    std::optional<Sound> Sound::Copy() const {
        if (m_Flags & StreamData){
            RC_WARN("Cannot copy sound intialized with StreamData flag");
            return std::nullopt;
        }

        Internal::SoundObject* internalSoundCopy = CreateShallowCopy(m_InternalSound, m_Flags, nullptr);

        if (!internalSoundCopy) {
            return std::nullopt;
        }

        return Sound(internalSoundCopy, m_Flags, nullptr);
    }
    
    std::optional<Sound> Sound::CopyDeep() const {
        if (m_Flags & StreamData){
            RC_WARN("Cannot copy sound intialized with StreamData flag");
            return std::nullopt;
        }

        ma_sound_group* group = m_Parent ? m_Parent->m_InternalBus.get() : nullptr;
        Internal::SoundObject* internalSoundCopy = CreateDeepCopy(m_InternalSound, m_Flags, group);

        if (!internalSoundCopy) {
            return std::nullopt;
        }
        
        return Sound(internalSoundCopy, m_Flags, m_Parent);
    }

    bool Sound::IsLooping() {
        return ma_sound_is_looping(m_InternalSound);
    }

    void Sound::SetLooping(bool looping) {
        ma_sound_set_looping(m_InternalSound, looping);
    }

    bool Sound::IsPlaying() {
        return ma_sound_is_playing(m_InternalSound);
    }

    bool Sound::IsAtEnd() {
        return ma_sound_at_end(m_InternalSound);
    }

    std::chrono::milliseconds Sound::GetTime() {
        return std::chrono::milliseconds(ma_sound_get_time_in_milliseconds(m_InternalSound));
    }

    std::optional<std::chrono::milliseconds> Sound::GetLength() {
        ma_uint64 length = 0;
        ma_result result = ma_sound_get_length_in_pcm_frames(m_InternalSound, &length);

        if (result != MA_SUCCESS) {
            RC_WARN("Getting length of sound, , failed with error ", (int32_t)result);
            return std::nullopt;
        }

        return std::chrono::milliseconds(length * 1000 / SAMPLERATE );
    }

    void Sound::Start(std::chrono::milliseconds fadeLength, float volumeMin, float volumeMax) {
        using namespace std::chrono_literals;
        
        if (GetFadeVolume() == 0.0f && m_FadeSettings.EndVolume == 0.0f) {
            SkipTo(0ms);
            if (fadeLength == 0ms) {
                SetFade(0ms, 1.0f, 1.0f);
            }
        }

        auto length = GetLength();
        if (length) {
            auto time = GetTime();
            if (time >= length.value()) {
                time %= length.value();
                SkipTo(time);
            }
        }

        if (fadeLength != 0ms) {
            SetFade(fadeLength, volumeMin, volumeMax);
        }

        ma_sound_start(m_InternalSound);
    }

    void Sound::Stop() {
        ma_sound_stop(m_InternalSound);
    }

    void Sound::Skip(std::chrono::milliseconds duration) {
        using namespace std::chrono_literals;
        
        if (duration == 0ms) {
            return;
        }

        ma_uint64 skipTo = 0;
        ma_result result = ma_sound_get_cursor_in_pcm_frames(m_InternalSound, &skipTo);
        
        if (result != MA_SUCCESS) {
            RC_WARN("Skipping sound forward {} ms failed with error ", duration, (int32_t)result);
            return;
        }

        skipTo += (SAMPLERATE * duration / 1s);
        ma_sound_seek_to_pcm_frame(m_InternalSound, skipTo);
    }

    void Sound::SkipTo(std::chrono::milliseconds timepoint) {
        using namespace std::chrono_literals;

        ma_uint64 skipTo = 0;
        if (timepoint != 0ms) {
            skipTo = (SAMPLERATE * timepoint / 1s);
        } 

        ma_sound_seek_to_pcm_frame(m_InternalSound, skipTo);
    }

    void Sound::SetVolume(float volume) {
        ma_sound_set_volume(m_InternalSound, volume);
    }

    void Sound::SetVolumedB(float dBVolume) {
        ma_sound_set_volume(m_InternalSound, ma_volume_db_to_linear(dBVolume));
    }

    void Sound::SetPitch(float pitch) {
        if (m_Flags & DisablePitch) {
            RC_WARN("Can not set pitch of Audio System sound created with DisablePitch flag");
            return;
        }

        ma_sound_set_pitch(m_InternalSound, pitch);
    }

    void Sound::SetBalance(float balance) {
        ma_sound_set_pan_mode(m_InternalSound, ma_pan_mode_balance);
        ma_sound_set_pan(m_InternalSound, balance);
    }

    void Sound::SetPan(float pan) {
        ma_sound_set_pan_mode(m_InternalSound, ma_pan_mode_pan);
        ma_sound_set_pan(m_InternalSound, pan);
    }
    
    void Sound::SetSpatialization(bool spatial) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set spatialization of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_spatialization_enabled(m_InternalSound, spatial);
    }
    
    void Sound::SetPositioning(Positioning positioning) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set positioning of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_positioning pos = positioning == Positioning::Absolute ? ma_positioning_absolute : ma_positioning_relative;

        ma_sound_set_positioning(m_InternalSound, pos);
    }
    
    void Sound::SetAttenuation(AttenuationMode attenuation) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set attentuation of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_attenuation_model att;
        switch (attenuation) {
        case AttenuationMode::None:
            att = ma_attenuation_model_none;
            break;
        case AttenuationMode::Inverse:
            att = ma_attenuation_model_inverse;
            break;
        case AttenuationMode::Linear:
            att = ma_attenuation_model_linear;
            break;
        case AttenuationMode::Exponential:
            att = ma_attenuation_model_exponential;
            break;
        default:
            att = ma_attenuation_model_none;
            break;
        }

        ma_sound_set_attenuation_model(m_InternalSound, att);
    }

    glm::vec3 Sound::GetDirectionToListner() {
        ma_vec3f direction = ma_sound_get_direction_to_listener(m_InternalSound);

        return glm::vec3(direction.x, direction.y, direction.z);
    }

    void Sound::SetSpatialData(glm::vec3 position, glm::vec3 direction, glm::vec3 velocity) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set spatial data of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_position(m_InternalSound, position.x, position.y, position.z);
        ma_sound_set_direction(m_InternalSound, direction.x, direction.y, direction.z);
        ma_sound_set_velocity(m_InternalSound, velocity.x, velocity.y, velocity.z);
    }

    void Sound::SetPosition(glm::vec3 position) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set position of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_position(m_InternalSound, position.x, position.y, position.z);
    }

    void Sound::SetDirection(glm::vec3 direction) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set direction of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_direction(m_InternalSound, direction.x, direction.y, direction.z);
    }
    
    void Sound::SetVelocity(glm::vec3 velocity) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set velocity of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_velocity(m_InternalSound, velocity.x, velocity.y, velocity.z);
    }
    
    void Sound::SetSpatialParameters(float rolloff, float gainMin, float gainMax, float distanceMin, float distanceMax, float coneInnerAngleRadians, float coneOuterAngleRadians, float coneOuterGain) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set spatial parameters of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_rolloff(m_InternalSound, rolloff);

        ma_sound_set_min_gain(m_InternalSound, gainMin);
        ma_sound_set_max_gain(m_InternalSound, gainMax);

        ma_sound_set_min_distance(m_InternalSound, distanceMin);
        ma_sound_set_max_distance(m_InternalSound, distanceMax);

        ma_sound_set_cone(m_InternalSound, coneInnerAngleRadians, coneOuterAngleRadians, coneOuterGain);
    }
    
    void Sound::SetRolloff(float rolloff) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set rolloff of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_rolloff(m_InternalSound, rolloff);
    }

    void Sound::SetGainRange(float min, float max) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set gain range of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_min_gain(m_InternalSound, min);
        ma_sound_set_max_gain(m_InternalSound, max);
    }

    void Sound::SetDistanceRange(float min, float max) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set distance range of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_min_distance(m_InternalSound, min);
        ma_sound_set_max_distance(m_InternalSound, max);
    }
    
    void Sound::SetCone(float innerAngleRadians, float outerAngleRadians, float outerGain) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set cone of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_cone(m_InternalSound, innerAngleRadians, outerAngleRadians, outerGain);
    }

    void Sound::SetConeDegrees(float innerAngleDegrees, float outerAngleDegrees, float outerGain) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set cone of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_cone(m_InternalSound, glm::radians(innerAngleDegrees), glm::radians(outerAngleDegrees), outerGain);
    }

    void Sound::SetSpatialFactors(float dopplerFactor, float directionalAttenuationFactor) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set spatial factors of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_doppler_factor(m_InternalSound, dopplerFactor);
        ma_sound_set_directional_attenuation_factor(m_InternalSound, directionalAttenuationFactor);
    }

    void Sound::SetDopplerFactor(float dopplerFactor) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set doppler factor of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_doppler_factor(m_InternalSound, dopplerFactor);
    }

    void Sound::SetDirectionalAttenuationFactor(float directionalAttenuationFactor) {
        if (m_Flags & DisableSpatialization) {
            RC_WARN("Can not set directional attenuation factor of Audio System sound created with DisableSpatialization flag");
            return;
        }

        ma_sound_set_directional_attenuation_factor(m_InternalSound, directionalAttenuationFactor);
    }

    float Sound::GetFadeVolume() {
        return ma_sound_get_current_fade_volume(m_InternalSound);
    }

    void Sound::SetFadeIn(std::chrono::milliseconds length, float endVolume, std::chrono::milliseconds startAfter) {
        SetFade(length, 0.0f, endVolume, startAfter);
    }

    void Sound::SetFadeOut(std::chrono::milliseconds length, std::chrono::milliseconds startAfter) {
        // If volume is not zero, start from the current volume
        if (GetFadeVolume()) {
            SetFade(length, -1.0f, 0.0f, startAfter);
            return;
        }

        SetFade(length, 1.0f, 0.0f, startAfter);
    }

    void Sound::SetFade(std::chrono::milliseconds length, float startVolume, float endVolume, std::chrono::milliseconds startAfter) {
        using namespace std::chrono_literals;
        
        ma_uint64 fadeInFrames = 0;
        if (length != 0ms) {
            fadeInFrames = SAMPLERATE * length / 1s;
        }
        
        if (startAfter != 0ms) {
            RC_ASSERT(Internal::System->Engine);
            ma_uint64 startTime = ma_engine_get_time_in_pcm_frames(Internal::System->Engine) + SAMPLERATE * startAfter / 1s;

            ma_sound_set_fade_start_in_pcm_frames(m_InternalSound, startVolume, endVolume, fadeInFrames, startTime);
            
            m_ScheduledFade = true;
            m_FadeSettings = { startTime, fadeInFrames, startVolume, endVolume };
            return;
        }

        ma_sound_set_fade_in_pcm_frames(m_InternalSound, startVolume, endVolume, fadeInFrames);
        m_ScheduledFade = false;
        m_FadeSettings = { ma_engine_get_time_in_pcm_frames(Internal::System->Engine), fadeInFrames, startVolume, endVolume };
    }

    void Sound::AttachParentBus(Bus& parent) {
        if (&parent == m_Parent) {
            return;
        }

        SwitchParent(&parent);

        // Sounds (ma_sound) have only one output bus and buses (ma_sound_group) have only one input bus, so index is always 0
        ma_result result = ma_node_attach_output_bus(m_InternalSound, 0, m_Parent->m_InternalBus.get(), 0);

        if (result != MA_SUCCESS) {
            RC_WARN("Attaching parent bus to sound failed with error {}", (int32_t)result);
        }
    }

    void Sound::SwitchParent(Bus* parent) {
        if (m_Parent) {
            m_Parent->DetachChild(this);
        }

        if (parent) {
            parent->AttachChild(this);
        }

        m_Parent = parent;
    }

    Sound::Flags::Flags(uint8_t flags) : Data(flags) {
        RC_ASSERT(Data < InvalidFlag);
    }
}