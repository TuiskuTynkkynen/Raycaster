#include "Audio.h"

#include "Internal.h"

#include "Core/Debug/Debug.h"

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ma_engine_read_pcm_frames(&Core::Audio::Internal::System->Engine, pOutput, frameCount, NULL);
}

void notification_callback(const ma_device_notification* pNotification) {
    RC_INFO("Notification type = {}", (uint32_t)pNotification->type);

    ma_device_info* devices;
    ma_uint32 count;
    ma_context_get_devices(&Core::Audio::Internal::System->Context, &devices, &count, nullptr, nullptr);
    
    size_t defaultDeviceIndex = 0;
    for (; defaultDeviceIndex < count && !devices[defaultDeviceIndex].isDefault; defaultDeviceIndex++) { }

    RC_INFO("default = {}, {}", defaultDeviceIndex, devices[defaultDeviceIndex].name);
    RC_INFO("current = {}", Core::Audio::Internal::System->Engine.pDevice->playback.name);
}

namespace Core::Audio {
    constexpr ma_format FORMAT = ma_format_f32;
    constexpr ma_uint32 SAMPLERATE = 48000;
    constexpr ma_uint32 CHANNELS = 0;
    constexpr ma_uint32 LISTNERS = 1;

    bool InitDevice(const ma_device_id* playbackId, const ma_device_data_proc dataCallback, const ma_device_notification_proc notificationCallback);
    static inline void ShutdownDevice(){ ma_device_uninit(&Internal::System->Device); }
    
    bool InitEngine();
    static inline void ShutdownEngine(){ ma_engine_uninit(&Internal::System->Engine); }
}

namespace Core {
	void Audio::Init() {
		RC_ASSERT(!Internal::System, "Audio System has already been initialized");
		Internal::System = std::make_unique<Internal::AudioSystem>();
        
        ma_resource_manager_config managerConfig = ma_resource_manager_config_init();
        managerConfig.decodedFormat = FORMAT;
        managerConfig.decodedChannels = CHANNELS;
        managerConfig.decodedSampleRate = SAMPLERATE;
        
        RC_ASSERT(ma_resource_manager_init(&managerConfig, &Internal::System->ResourceManager) == MA_SUCCESS, "Failed to initialize Audio System. Could not initialize resource manager.\n");
	
        RC_ASSERT(ma_context_init(nullptr, 0, nullptr, &Internal::System->Context) == MA_SUCCESS, "Failed to initialize Audio System. Could not initialize context.\n");
        RC_ASSERT(Audio::InitDevice(nullptr, data_callback, notification_callback), "Failed to intitialize Audio System");
        
        RC_ASSERT(Audio::InitEngine(), "Failed to intitialize Audio System");
    }
	
	void Audio::Shutdown() {
		RC_ASSERT(Internal::System, "Audio System has not been initialized");
		Internal::System.reset();

        ShutdownEngine();
        
        ShutdownDevice();
		ma_context_uninit(&Internal::System->Context);

        ma_resource_manager_uninit(&Internal::System->ResourceManager);
    }

	void Audio::Play(const char* filePath) {
		RC_ASSERT(Internal::System, "Audio System has not been initialized");

		ma_result result = ma_engine_play_sound(&Internal::System->Engine, filePath, NULL);
	}
    
    bool Audio::InitDevice(const ma_device_id* playbackId, const ma_device_data_proc dataCallback, const ma_device_notification_proc notificationCallback) {
        if (ma_device__is_initialized(&Internal::System->Device)) {
            RC_ERROR("Tried to intialize already initialized device");
            return false;
        }
        
        ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);

        deviceConfig.playback.format = FORMAT;
        deviceConfig.playback.channels = CHANNELS;
        deviceConfig.sampleRate = SAMPLERATE;

        deviceConfig.dataCallback = dataCallback;
        deviceConfig.notificationCallback = notificationCallback;
        deviceConfig.playback.pDeviceID = playbackId;

        ma_result result = ma_device_init(&Internal::System->Context, &deviceConfig, &Internal::System->Device);
        if (result != MA_SUCCESS) {
            RC_ERROR("Failed to initialize device with error {}", (int32_t)result);
            return false;
        }

        return true;
    }

    bool Audio::InitEngine() {
        ma_engine_config engineConfig = ma_engine_config_init();
        engineConfig.channels = CHANNELS;
        engineConfig.sampleRate = SAMPLERATE;
        engineConfig.listenerCount = LISTNERS;

        engineConfig.pContext = &Internal::System->Context;
        engineConfig.pDevice = &Internal::System->Device;
        engineConfig.pResourceManager = &Internal::System->ResourceManager;
        
        ma_result result = ma_engine_init(&engineConfig, &Internal::System->Engine);
        if (result != MA_SUCCESS) {
            RC_ERROR("Failed to initialize engine with error {}", (int32_t)result);
            return false;
        }
        
        return true;
    }
}