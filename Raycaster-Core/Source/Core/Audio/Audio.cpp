#include "Audio.h"

#include "Internal.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

#include "Core/Debug/Debug.h"

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    if (!Core::Audio::Internal::System->Engine) {
        return;
    }
    
    ma_engine_read_pcm_frames(Core::Audio::Internal::System->Engine, pOutput, frameCount, NULL);
}

void notification_callback(const ma_device_notification* pNotification) {
    if (!Core::Audio::Internal::System->Engine) {
        return;
    }
    
    RC_INFO("Notification type = {}", (uint32_t)pNotification->type);

    ma_device_info* devices;
    ma_uint32 count;
    ma_context_get_devices(&Core::Audio::Internal::System->Context, &devices, &count, nullptr, nullptr);

    size_t defaultDeviceIndex = 0;
    for (; defaultDeviceIndex < count && !devices[defaultDeviceIndex].isDefault; defaultDeviceIndex++) { }

    RC_INFO("default = {}, {}", defaultDeviceIndex, devices[defaultDeviceIndex].name);
    RC_INFO("current = {}", Core::Audio::Internal::System->Engine->pDevice->playback.name);
}

namespace Core::Audio {
    bool InitDevice(const ma_device_id* playbackId, const ma_device_data_proc dataCallback, const ma_device_notification_proc notificationCallback);
    
    static inline void ShutdownDevice() { 
        ma_device_uninit(Internal::System->Device);
        delete Internal::System->Device;
        Internal::System->Device = nullptr;
    }

    static inline void ShutdownDevice(ma_device* device) { 
        ma_device_uninit(device);
        delete device; 
    }

    void SetInternalDevice(const ma_device_id* playbackId);

    bool InitEngine();
    
    static inline void ShutdownEngine() { 
        ma_engine_uninit(Internal::System->Engine);
        delete Internal::System->Engine;
        Internal::System->Engine = nullptr;
    }

    static inline void ShutdownEngine(ma_engine* engine) { 
        ma_engine_uninit(engine);
        delete engine;
    }
}

namespace Core {
    void Audio::Init() {
        RC_ASSERT(!Internal::System, "Audio System has already been initialized");
        Internal::System = std::make_unique<Internal::AudioSystem>();

        ma_resource_manager_config managerConfig = ma_resource_manager_config_init();
        managerConfig.decodedFormat = FORMAT;
        managerConfig.decodedChannels = CHANNELS;
        managerConfig.decodedSampleRate = SAMPLERATE;

        bool success = ma_resource_manager_init(&managerConfig, &Internal::System->ResourceManager) == MA_SUCCESS;
        RC_ASSERT(success, "Failed to initialize Audio System. Could not initialize resource manager.\n");

        success = ma_context_init(nullptr, 0, nullptr, &Internal::System->Context) == MA_SUCCESS;
        RC_ASSERT(success, "Failed to initialize Audio System. Could not initialize context.\n");

        success = Audio::InitDevice(nullptr, data_callback, notification_callback);
        RC_ASSERT(success, "Failed to intitialize Audio System");

        success = Audio::InitEngine();
        RC_ASSERT(success, "Failed to intitialize Audio System");
    }

    void Audio::Shutdown() {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        ShutdownEngine();

        ShutdownDevice();
        ma_context_uninit(&Internal::System->Context);

        ma_resource_manager_uninit(&Internal::System->ResourceManager);

        Internal::System.reset();
    }

    void Audio::PlayInlineSound(std::string_view filePath) {
        std::filesystem::path path = filePath;
        if (path.is_relative()) {
            path = std::filesystem::current_path() / "Source" / "Audio" / path;
        }

        PlayInlineSound(path);
    }

    void Audio::PlayInlineSound(std::filesystem::path& path) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        std::string pathString = path.string();

        ma_result result = ma_engine_play_sound(Internal::System->Engine, pathString.c_str(), NULL);
        if (result != MA_SUCCESS) {
            RC_WARN("Playing inline sound, {}, failed with error code {}", pathString, (int32_t)result);
        }
    }

    void Audio::Play(SoundManager::Index index) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        Sound* sound = Internal::System->SoundManager.GetSound(index);

        if (!sound) {
            return;
        }

        sound->Start();
    }

    void Audio::Play(SoundManager::Index& index, std::string_view name) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        index = Internal::System->SoundManager.ValidateIndex(index, name);
        Play(index);
    }

    void Audio::Play(std::string_view name) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        Sound* sound = Internal::System->SoundManager.GetSound(name);

        if (!sound) {
            return;
        }

        sound->Start();
    }

    void Audio::Pause(SoundManager::Index index) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        Sound* sound = Internal::System->SoundManager.GetSound(index);

        if (!sound) {
            return;
        }

        sound->Stop();
    }

    void Audio::Pause(SoundManager::Index& index, std::string_view name) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        index = Internal::System->SoundManager.ValidateIndex(index, name);
        Pause(index);
    }

    void Audio::Pause(std::string_view name) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        Sound* sound = Internal::System->SoundManager.GetSound(name);

        if (!sound) {
            return;
        }

        sound->Stop();
    }

    void Audio::Stop(SoundManager::Index index) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        Sound* sound = Internal::System->SoundManager.GetSound(index);

        if (!sound) {
            return;
        }

        sound->Stop();
        sound->SkipTo(std::chrono::milliseconds::zero());
    }

    void Audio::Stop(SoundManager::Index& index, std::string_view name) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        index = Internal::System->SoundManager.ValidateIndex(index, name);
        Stop(index);
    }

    void Audio::Stop(std::string_view name) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        Sound* sound = Internal::System->SoundManager.GetSound(name);

        if (!sound) {
            return;
        }

        sound->Stop();
        sound->SkipTo(std::chrono::milliseconds::zero());
    }

    Audio::SoundManager& Audio::GetSoundManager() {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        return Internal::System->SoundManager;
    }

    bool Audio::ValidateSoundIndex(SoundManager::Index& index, std::string_view name) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        index = Internal::System->SoundManager.ValidateIndex(index, name);
        return index;
    }

    Audio::SoundManager::Index Audio::GetSoundIndex(std::string_view name) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        return Internal::System->SoundManager.GetSoundIndex(name);
    }

    Audio::Sound* Audio::GetSound(SoundManager::Index index) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        return Internal::System->SoundManager.GetSound(index);
    }

    Audio::Sound* Audio::GetSound(std::string_view name) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        return Internal::System->SoundManager.GetSound(name);
    }

    std::vector<std::string_view> Audio::GetDevices() {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        Internal::System->AvailableDevices.clear();

        uint32_t playbackDeviceCount = 0;
        auto enumerationCallback = [](ma_context* pContext, ma_device_type deviceType, const ma_device_info* pInfo, void* pUserData) -> ma_bool32 {
            if (deviceType != ma_device_type_playback) {
                return false;
            }

            Internal::System->AvailableDevices.emplace_back(pInfo->id);

            for (size_t i = 0; pInfo->name[i]; i++) {
                Internal::System->AvailableDevices.back().name[i] = pInfo->name[i];
            }

            uint32_t* count = (uint32_t*)pUserData;
            (*count)++;

            return true;
            };

        ma_result result = ma_context_enumerate_devices(&Internal::System->Context, enumerationCallback, &playbackDeviceCount);
        if (result != MA_SUCCESS) {
            RC_WARN("Retreiving device information failed with error code {}", (int32_t)result);
        }

        std::vector<std::string_view> names;
        names.reserve(playbackDeviceCount);

        for (size_t i = 0; i < playbackDeviceCount; i++) {
            names.emplace_back(Internal::System->AvailableDevices[i].name);
        }

        return names;
    }

    void Audio::SetInternalDevice(const ma_device_id* playbackId) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        // Copy internal device and engine pointers and set them to nullptr
        ma_device* oldDevice = std::exchange(Internal::System->Device, nullptr);
        ma_engine* oldEngine = std::exchange(Internal::System->Engine, nullptr);

        //Create new engine and device
        bool result = InitDevice(playbackId, data_callback, notification_callback) && InitEngine();
        RC_ASSERT(result, "Audio System SetDevice failed");

        Internal::System->SoundManager.ReinitSounds();

        //Shutdown old engine and device
        ShutdownEngine(oldEngine);
        ShutdownDevice(oldDevice);
    }

    void Audio::SetDevice(size_t deviceIndex) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        if (deviceIndex >= Internal::System->Context.playbackDeviceInfoCount) {
            RC_WARN("Audio System SetDevice() called with invalid device index");
            return;
        }

        SetInternalDevice(&Internal::System->Context.pDeviceInfos[deviceIndex].id);
    }

    void Audio::SetDevice(std::string_view deviceName) {
        RC_ASSERT(Internal::System, "Audio System has not been initialized");

        size_t deviceIndex = -1;
        for (size_t i = 0; i < Internal::System->AvailableDevices.size(); i++) {
            if (Internal::System->AvailableDevices[i].name == deviceName) {
                deviceIndex = i;
                break;
            }
        }

        if (deviceIndex >= Internal::System->Context.playbackDeviceInfoCount) {
            RC_WARN("Audio System SetDevice() called with invalid device name");
            return;
        }

        SetInternalDevice(&Internal::System->Context.pDeviceInfos[deviceIndex].id);
    }

    // Switch to default device
    void Audio::SetDevice() {
        SetInternalDevice(nullptr);
    }

    bool Audio::InitDevice(const ma_device_id* playbackId, const ma_device_data_proc dataCallback, const ma_device_notification_proc notificationCallback) {
        if (Internal::System->Device) {
            RC_ERROR("Tried to intialize already initialized device");
            return false;
        }

        Internal::System->Device = new ma_device;

        ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);

        deviceConfig.playback.format = FORMAT;
        deviceConfig.playback.channels = CHANNELS;
        deviceConfig.sampleRate = SAMPLERATE;

        deviceConfig.dataCallback = dataCallback;
        deviceConfig.notificationCallback = notificationCallback;
        deviceConfig.playback.pDeviceID = playbackId;

        ma_result result = ma_device_init(&Internal::System->Context, &deviceConfig, Internal::System->Device);
        if (result != MA_SUCCESS) {
            RC_ERROR("Failed to initialize device with error {}", (int32_t)result);
            return false;
        }

        return true;
    }

    bool Audio::InitEngine() {
        if (Internal::System->Engine) {
            RC_ERROR("Tried to intialize already initialized engine");
            return false;
        }

        RC_ASSERT(Internal::System->Device, "Tried to initialize engine before device")

        Internal::System->Engine = new ma_engine;

        ma_engine_config engineConfig = ma_engine_config_init();
        engineConfig.channels = CHANNELS;
        engineConfig.sampleRate = SAMPLERATE;
        engineConfig.listenerCount = LISTNERS;

        engineConfig.pContext = &Internal::System->Context;
        engineConfig.pDevice = Internal::System->Device;
        engineConfig.pResourceManager = &Internal::System->ResourceManager;

        ma_result result = ma_engine_init(&engineConfig, Internal::System->Engine);
        if (result != MA_SUCCESS) {
            RC_ERROR("Failed to initialize engine with error {}", (int32_t)result);
            return false;
        }

        return true;
    }
}