#include "Bus.h"

#include "Internal.h"
#include "Core/Debug/Debug.h"

struct InternalBusObject : ma_sound_group {
    ~InternalBusObject() {
        ma_sound_group_uninit(this);
    }
};

namespace Core::Audio {
    Bus::Bus() {
        RC_ASSERT(Internal::System, "Tried to create bus before initializing Audio System");

        m_InternalBus = std::make_unique<InternalBusObject>();
        ma_result result = ma_sound_group_init(Internal::System->Engine, 0, nullptr, m_InternalBus.get());

        if (result != MA_SUCCESS) {
            RC_WARN("Initializing bus failed with error {}", (int32_t)result);
        }
    }

    Bus::Bus(const Bus& parent) {
        RC_ASSERT(Internal::System, "Tried to create bus before initializing Audio System");

        m_InternalBus = std::make_unique<InternalBusObject>();
        ma_result result = ma_sound_group_init(Internal::System->Engine, 0, parent.m_InternalBus.get(), m_InternalBus.get());

        if (result != MA_SUCCESS) {
            RC_WARN("Initializing bus failed with error {}", (int32_t)result);
        }
    }

    // Needs to be defined in source file so std::unique_ptr<InternalBusObject> m_InternalBus can call the InternalBusObject destuctor
    Bus::Bus(Bus&& other) noexcept : m_InternalBus(std::exchange(other.m_InternalBus, nullptr)) {}

    // Needs to be defined in source file so std::unique_ptr<InternalBusObject> m_InternalBus can call the InternalBusObject destuctor
    Bus::~Bus() {} 

    bool Bus::IsEnabled() {
        return ma_sound_group_is_playing(m_InternalBus.get());
    }

    void Bus::SetEnabled(bool enabled) {
        ma_result result;

        if (enabled) {
            // Clear possibly set stop time. Feels hacky, but miniaudio does the same during sound/group/node initialization
            ma_sound_group_set_stop_time_in_pcm_frames(m_InternalBus.get(), -1);

            result = ma_sound_group_start(m_InternalBus.get());
        } else {
            result = ma_sound_group_stop(m_InternalBus.get());
        }

        if (result != MA_SUCCESS) {
            RC_WARN("Setting bus enabled failed with error {}", (int32_t)result);
        }
    }

    void Bus::SetEnabled(bool enabled, std::chrono::milliseconds delay) {
        using namespace std::chrono_literals;

        if (delay <= 0ms) {
            SetEnabled(enabled);
        }

        RC_ASSERT(Internal::System->Engine);

        if (enabled) {
            // Clear possibly set stop time. Feels hacky, but miniaudio does the same during node initialization
            ma_sound_group_set_stop_time_in_pcm_frames(m_InternalBus.get(), -1);

            ma_uint64 startTime = ma_engine_get_time_in_pcm_frames(Internal::System->Engine) + SAMPLERATE * delay / 1s;
            ma_sound_group_set_start_time_in_pcm_frames(m_InternalBus.get(), startTime);

            ma_result result = ma_sound_group_start(m_InternalBus.get());
            if (result != MA_SUCCESS) {
                RC_WARN("Setting bus enabled, with delay = {}, failed with error {}", delay, (int32_t)result);
            }
        } else {
            ma_uint64 stopTime = ma_engine_get_time_in_pcm_frames(Internal::System->Engine) + SAMPLERATE * delay / 1s;
            ma_sound_group_set_stop_time_in_pcm_frames(m_InternalBus.get(), stopTime);
        }
    }

    float Bus::GetVolume() {
        return ma_sound_group_get_volume(m_InternalBus.get());
    }

    void Bus::SetVolume(float volume) {
        ma_sound_group_set_volume(m_InternalBus.get(), volume);
    }

    float Bus::GetGaindB() {
        return ma_volume_linear_to_db(ma_sound_group_get_volume(m_InternalBus.get()));
    }

    void Bus::SetGain(float gaindB) {
        ma_sound_group_set_volume(m_InternalBus.get(), ma_volume_db_to_linear(gaindB));
    }

    float Bus::GetPitch() {
        return ma_sound_group_get_pitch(m_InternalBus.get());

    }

    void Bus::SetPitch(float pitch) {
        ma_sound_group_set_pitch(m_InternalBus.get(), pitch);
    }

    std::variant<Bus::Balance, Bus::Pan> Bus::GetBalanceOrPan() {
        switch (ma_sound_group_get_pan_mode(m_InternalBus.get())) {
        case ma_pan_mode_balance:
            return Balance(ma_sound_group_get_pan(m_InternalBus.get()));
        case ma_pan_mode_pan:
            return Pan(ma_sound_group_get_pan(m_InternalBus.get()));
        default:
            RC_ASSERT(false, "Assumes pan mode is balance or pan");
            return Balance();
        }
    }

    void Bus::SetBalance(float balance) {
        ma_sound_group_set_pan_mode(m_InternalBus.get(), ma_pan_mode_balance);
        ma_sound_group_set_pan(m_InternalBus.get(), balance);
    }

    void Bus::SetPan(float pan) {
        ma_sound_group_set_pan_mode(m_InternalBus.get(), ma_pan_mode_pan);
        ma_sound_group_set_pan(m_InternalBus.get(), pan);
    }

    float Bus::GetFadeVolume() {
        return ma_sound_group_get_current_fade_volume(m_InternalBus.get());
    }

    void Bus::SetFadeIn(std::chrono::milliseconds length) {
        SetFade(length, 0.0f, 1.0f);
    }

    void Bus::SetFadeOut(std::chrono::milliseconds length) {
        // If volume is not zero, start from the current volume
        if (GetFadeVolume()) {
            SetFade(length, -1.0f, 0.0f);
            return;
        }

        SetFade(length, 1.0f, 0.0f);
    }

    void Bus::SetFade(std::chrono::milliseconds length, float startVolume, float endVolume) {
        using namespace std::chrono_literals;

        ma_uint64 fadeInFrames = 0;
        if (length != 0ms) {
            fadeInFrames = SAMPLERATE * length / 1s;
        }

        ma_sound_group_set_fade_in_pcm_frames(m_InternalBus.get(), startVolume, endVolume, fadeInFrames);
    }

    void Bus::AttachParentBus(const Bus& parent) {
        // Buses (ma_sound_group) have only one input and output bus, so index is always 0
        ma_result result = ma_node_attach_output_bus(m_InternalBus.get(), 0, parent.m_InternalBus.get(), 0);

        if (result != MA_SUCCESS) {
            RC_WARN("Attaching parent bus to bus failed with error {}", (int32_t)result);
        }
    }
}