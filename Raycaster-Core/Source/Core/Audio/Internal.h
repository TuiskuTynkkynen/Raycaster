#pragma once

#include "Types.h"
#include "SoundManager.h"
#include "BusManager.h"

#include "miniaudio/miniaudio.h"

#include <memory>

namespace Core::Audio::Internal {
    struct AudioSystem {
        ma_resource_manager ResourceManager{};
        ma_context Context{};
        ma_device* Device = nullptr;
        ma_engine* Engine = nullptr;
        
        SoundManager SoundManager;
        BusManager BusManager;

        struct DeviceInfo {
            ma_device_id id;
            char name[MA_MAX_DEVICE_NAME_LENGTH + 1];   /* +1 for null terminator. */
        };
        std::vector<DeviceInfo> AvailableDevices;
    };

    inline std::unique_ptr<AudioSystem> System;
}

namespace Core::Audio {
        constexpr ma_format FORMAT = ma_format_f32;
        constexpr ma_uint32 SAMPLERATE = 48000;
        constexpr ma_uint32 CHANNELS = 0;
        constexpr ma_uint32 LISTNERS = 1;
}

namespace Core::Audio::Internal {
    struct SoundObject : ma_sound {};

    struct BusObject : ma_sound_group {
        ~BusObject() {
            ma_sound_group_uninit(this);
        }
    };
}