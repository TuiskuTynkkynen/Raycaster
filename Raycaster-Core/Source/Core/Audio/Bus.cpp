#include "Bus.h"

#include "Internal.h"
#include "Sound.h"
#include "Core/Debug/Debug.h"

namespace Core::Audio {
    Bus::Bus() {
        RC_ASSERT(Internal::System, "Tried to create bus before initializing Audio System");

        m_InternalBus = std::make_unique<Internal::BusObject>();
        ma_result result = ma_sound_group_init(Internal::System->Engine, 0, nullptr, m_InternalBus.get());

        if (result != MA_SUCCESS) {
            RC_WARN("Initializing bus failed with error {}", (int32_t)result);
        }

        m_Parent = nullptr;
    }

    Bus::Bus(Bus& parent) {
        RC_ASSERT(Internal::System, "Tried to create bus before initializing Audio System");

        m_InternalBus = std::make_unique<Internal::BusObject>();
        ma_result result = ma_sound_group_init(Internal::System->Engine, 0, parent.m_InternalBus.get(), m_InternalBus.get());

        if (result != MA_SUCCESS) {
            RC_WARN("Initializing bus failed with error {}", (int32_t)result);
        }

        m_Parent = nullptr;
        SwitchParent(&parent);
    }

    // Needs to be defined in source file so std::unique_ptr<Internal::BusObject> m_InternalBus can call the Internal::BusObject destuctor
    Bus::Bus(Bus&& other) noexcept : m_Parent(nullptr) {
        m_InternalBus.swap(other.m_InternalBus);
        m_Children.swap(other.m_Children);
        m_FadeSettings = other.m_FadeSettings;

        SwitchParent(other.m_Parent);
        other.SwitchParent(nullptr);

        for (auto& variant : m_Children) {
            std::visit([this](auto& child) { child->SwitchParent(this); }, variant);
    }
    }

    // Needs to be defined in source file so std::unique_ptr<Internal::BusObject> m_InternalBus can call the Internal::BusObject destuctor
    Bus::~Bus() {
        SwitchParent(nullptr);

        for (auto& variant : m_Children) {
            std::visit([](auto& child) { child->SwitchParent(nullptr); }, variant);
        }
    }

    void Bus::Reinit() {
        Internal::BusObject* copy = new Internal::BusObject;

        {
            ma_sound_group* parent = m_Parent ? m_Parent->m_InternalBus.get() : nullptr;

            ma_result result = ma_sound_group_init(Internal::System->Engine, 0, parent, copy);
            if (result != MA_SUCCESS) {
                RC_WARN("Reinitializing bus failed with error {}", (int32_t)result);
                delete copy;
                return;
            }
        }

        ma_sound_set_volume(copy, GetVolume());
        ma_sound_set_pitch(copy, GetPitch());

        Internal::BusObject* original = m_InternalBus.get();

        ma_sound_set_pan_mode(copy, ma_sound_get_pan_mode(original));
        ma_sound_set_pan(copy, ma_sound_get_pan(original));

        if (m_FadeSettings.StartVolume != 1.0f || m_FadeSettings.EndVolume != 1.0f) {
            RC_ASSERT(Internal::System->Engine);

            ma_uint64 currentTime = ma_engine_get_time_in_pcm_frames(Internal::System->Engine);
            ma_uint64 oldEngineTime = ma_engine_get_time_in_pcm_frames(m_InternalBus->engineNode.pEngine);
            ma_int64 relativeStartTime = m_FadeSettings.StartTime - oldEngineTime;

            m_FadeSettings.StartTime = currentTime;

            if (relativeStartTime < 0) { // Fade has started, so use update values
                m_FadeSettings.Length = currentTime + relativeStartTime;
                m_FadeSettings.StartVolume = GetFadeVolume();
            }

            ma_sound_group_set_fade_in_pcm_frames(original, m_FadeSettings.StartVolume, m_FadeSettings.EndVolume, m_FadeSettings.Length);
        }

        // ma_sound_group is enabled/started by default
        if (!IsEnabled()) {
            ma_sound_group_stop(copy);
        }

        {
            ma_uint64 oldEngineTime = ma_engine_get_time_in_pcm_frames(ma_sound_group_get_engine(original));
            ma_uint64 newEngineTime = ma_engine_get_time_in_pcm_frames(Internal::System->Engine);

            ma_uint64 startTime = ma_node_get_state_time(original, ma_node_state_started);
            // Only case where state will be changed in the future needs to be considered
            if (startTime > oldEngineTime) {
                ma_sound_group_set_start_time_in_pcm_frames(copy, startTime - oldEngineTime + newEngineTime);
                ma_sound_group_start(copy);
            }

            ma_uint64 stopTime = ma_node_get_state_time(original, ma_node_state_stopped);
            // Only case where state will be changed in the future needs to be considered
            // (ma_uint64)-1 used as default value by miniaudio and should not be changed
            if (stopTime != (ma_uint64)-1 && stopTime > oldEngineTime) {
                ma_sound_group_set_stop_time_in_pcm_frames(copy, stopTime - oldEngineTime + newEngineTime);
            }
        }

        m_InternalBus.reset(copy);
    }

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

        RC_ASSERT(Internal::System->Engine);
        m_FadeSettings = { ma_engine_get_time_in_pcm_frames(Internal::System->Engine), fadeInFrames, startVolume, endVolume };
    }

    void Bus::AttachParentBus(Bus& parent) {
        if (&parent == m_Parent) {
            return;
        }

        SwitchParent(&parent);

        // Buses (ma_sound_group) have only one input and output bus, so index is always 0
        ma_result result = ma_node_attach_output_bus(m_InternalBus.get(), 0, m_Parent->m_InternalBus.get(), 0);

        if (result != MA_SUCCESS) {
            RC_WARN("Attaching parent bus to bus failed with error {}", (int32_t)result);
        }
    }

    void Bus::SwitchParent(Bus* parent) {
        if (m_Parent) {
            m_Parent->DetachChild(this);
        }

        if (parent) {
            parent->AttachChild(this);
        }

        m_Parent = parent;
    }


    void Bus::AttachChild(ChildNode child) {
        m_Children.push_back(child);
    }

    void Bus::DetachChild(ChildNode child) {
        auto iter = std::find(m_Children.begin(), m_Children.end(), child);

        if (iter == m_Children.end()) {
            return;
        }

        iter->swap(m_Children.back());
        m_Children.pop_back();
    }
}