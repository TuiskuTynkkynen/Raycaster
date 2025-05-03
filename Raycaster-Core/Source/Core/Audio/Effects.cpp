#include "Effects.h"

#include "Internal.h"
#include "Core/Debug/Debug.h"

namespace Core::Audio::Effects {
    Internal::FilterNode CreateFilterNode(auto settings, ma_node* parent);

    Internal::FilterNode CreateFilterNode(auto settings, ma_node* parent) {
        static_assert(false, "Unsupported type passed as settings");
    }

    static bool ReinitFilterNode(auto* node, ma_node* parent);

    static bool ReinitFilterNode(auto* node, ma_node* parent) {
        static_assert(false, "Pointer to unsupported type passed as node");
    }
    
    static void UninitFilterNode(auto* node);

    static void UninitFilterNode(auto* node) {
        static_assert(false, "Pointer to unsupported type passed as node");
    }

    static constexpr FilterType FilterNodeToType(auto* node);

    static constexpr FilterType FilterNodeToType(auto* node) {
        static_assert(false, "Pointer to unsupported type passed as node");
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Delay Filter Node                                                                                                                 //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    static Internal::FilterNode CreateFilterNode(Effects::DelaySettings settings, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        ma_uint32 channels = ma_engine_get_channels(Internal::System->Engine);

        RC_ASSERT(settings.Length > std::chrono::milliseconds::zero(), "Delay must be > 0");

        ma_uint32 delayInFrames = 0;
        {
            using namespace std::chrono_literals;
            if (settings.Length != 0ms) {
                delayInFrames = static_cast<ma_uint32>(SAMPLERATE * settings.Length / 1s);
            }
        }

        ma_delay_node_config config = ma_delay_node_config_init(channels, SAMPLERATE, delayInFrames, settings.Decay);

        config.delay.dry = settings.Dry;
        config.delay.wet = settings.Wet;
        config.delay.delayStart = (!settings.Decay) ? true : settings.DelaySoundStart;

        Internal::Delay* node = new Internal::Delay;

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_delay_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Creating delay filter node failed with error code {}", (int32_t)result);

            delete node;
            node = nullptr;
            return node;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching delay filter node to parent failed with error code {}", (int32_t)result);
        }

        return node;
    }

    template<>
    static bool ReinitFilterNode(Internal::Delay* node, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);

        ma_delay_node_config config = ma_delay_node_config_init(node->delay.config.channels, node->delay.config.sampleRate, node->delay.config.delayInFrames, node->delay.config.decay);

        config.delay.dry = node->delay.config.dry;
        config.delay.wet = node->delay.config.wet;
        config.delay.delayStart = node->delay.config.delayStart;

        ma_delay_node_uninit(node, nullptr);
        
        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_delay_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Reinitializing delay filter node failed with error code {}", (int32_t)result);

            delete node;
            node = nullptr;
            return false;
        }

        if (!parent) {
            return true;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching delay filter node to parent failed with error code {}", (int32_t)result);
        }

        return true;
    }

    template<>
    static void UninitFilterNode(Internal::Delay* node) {
        ma_delay_node_uninit(node, nullptr);
        delete node;
    }

    static constexpr FilterType FilterNodeToType(Internal::Delay* node) {
        return FilterType::Delay;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Biquad Filter Node                                                                                                                 //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    static Internal::FilterNode CreateFilterNode(Effects::BiquadSettings settings, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        ma_uint32 channels = ma_engine_get_channels(Internal::System->Engine);

        ma_biquad_node_config config = ma_biquad_node_config_init(channels, settings.B[0], settings.B[1], settings.B[2], settings.A[0], settings.A[1], settings.A[2]);

        Internal::Biquad* node = new Internal::Biquad;

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_biquad_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Creating biquad filter node failed with error code {}", (int32_t)result);

            delete node;
            node = nullptr;
            return node;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching biquad filter node to parent failed with error code {}", (int32_t)result);
        }

        return node;
    }

    static bool ReinitFilterNode(Internal::Biquad* node, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        
        RC_ASSERT(node->biquad.format == ma_format_f32, "Only float32 format is supported");
        ma_biquad_node_config config = ma_biquad_node_config_init(node->biquad.channels, node->biquad.b0.f32, node->biquad.b1.f32, node->biquad.b2.f32, 1.0f, node->biquad.a1.f32, node->biquad.a2.f32);

        ma_biquad_node_uninit(node, nullptr);

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_biquad_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Reinitializing biquad filter node failed with error code {}", (int32_t)result);
        
            delete node;
            node = nullptr;
            return false;
        }

        if (!parent) {
            return true;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching biquad filter node to parent failed with error code {}", (int32_t)result);
        }
        
        return true;
    }

    template<>
    static void UninitFilterNode(Internal::Biquad* node) {
        ma_biquad_node_uninit(node, nullptr);
        delete node;
    }

    static constexpr FilterType FilterNodeToType(Internal::Biquad* node) {
        return FilterType::Biquad;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Low Pass Filter Node                                                                                                              //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    static Internal::FilterNode CreateFilterNode(Effects::LowPassSettings settings, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        ma_uint32 channels = ma_engine_get_channels(Internal::System->Engine);

        ma_lpf_node_config config = ma_lpf_node_config_init(channels, SAMPLERATE, settings.CutoffFrequency, settings.Order);

        Internal::LowPass* node = new Internal::LowPass;
        node->CutoffFrequency = settings.CutoffFrequency;

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_lpf_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Creating low pass filter node failed with error code {}", (int32_t)result);

            delete node;
            node = nullptr;
            return node;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching low pass filter node to parent failed with error code {}", (int32_t)result);
        }

        return node;
    }

    static bool ReinitFilterNode(Internal::LowPass* node, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        
        ma_uint32 order = node->lpf.lpf1Count + 2 * node->lpf.lpf2Count;
        double cutoff = node->CutoffFrequency;
        
        ma_lpf_node_config config = ma_lpf_node_config_init(node->lpf.channels, node->lpf.sampleRate, cutoff, order);

        ma_lpf_node_uninit(node, nullptr);

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_lpf_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Reinitializing low pass filter node failed with error code {}", (int32_t)result);
        
            delete node;
            node = nullptr;
            return false;
        }

        if (!parent) {
            return true;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching low pass filter node to parent failed with error code {}", (int32_t)result);
        }
        
        return true;
    }

    template<>
    static void UninitFilterNode(Internal::LowPass* node) {
        ma_lpf_node_uninit(node, nullptr);
        delete node;
    }

    static constexpr FilterType FilterNodeToType(Internal::LowPass* node) {
        return FilterType::LowPass;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // High Pass Filter Node                                                                                                             //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    static Internal::FilterNode CreateFilterNode(Effects::HighPassSettings settings, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        ma_uint32 channels = ma_engine_get_channels(Internal::System->Engine);

        ma_hpf_node_config config = ma_hpf_node_config_init(channels, SAMPLERATE, settings.CutoffFrequency, settings.Order);

        Internal::HighPass* node = new Internal::HighPass;
        node->CutoffFrequency = settings.CutoffFrequency;

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_hpf_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Creating high pass filter node failed with error code {}", (int32_t)result);

            delete node;
            node = nullptr;
            return node;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching high pass filter node to parent failed with error code {}", (int32_t)result);
        }

        return node;
    }

    static bool ReinitFilterNode(Internal::HighPass* node, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        
        ma_uint32 order = node->hpf.hpf1Count + 2 * node->hpf.hpf2Count;
        double cutoff = node->CutoffFrequency;
        
        ma_hpf_node_config config = ma_hpf_node_config_init(node->hpf.channels, node->hpf.sampleRate, cutoff, order);

        ma_hpf_node_uninit(node, nullptr);

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_hpf_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Reinitializing high pass filter node failed with error code {}", (int32_t)result);
        
            delete node;
            node = nullptr;
            return false;
        }

        if (!parent) {
            return true;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching high pass filter node to parent failed with error code {}", (int32_t)result);
        }
        
        return true;
    }

    template<>
    static void UninitFilterNode(Internal::HighPass* node) {
        ma_hpf_node_uninit(node, nullptr);
        delete node;
    }

    static constexpr FilterType FilterNodeToType(Internal::HighPass* node) {
        return FilterType::HighPass;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Band Pass Filter Node                                                                                                             //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    static Internal::FilterNode CreateFilterNode(Effects::BandPassSettings settings, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        ma_uint32 channels = ma_engine_get_channels(Internal::System->Engine);

        if (settings.Order % 2 != 0) {
            settings.Order--;
        }

        ma_bpf_node_config config = ma_bpf_node_config_init(channels, SAMPLERATE, settings.CutoffFrequency, settings.Order);

        Internal::BandPass* node = new Internal::BandPass;
        node->CutoffFrequency = settings.CutoffFrequency;

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_bpf_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Creating band pass filter node failed with error code {}", (int32_t)result);

            delete node;
            node = nullptr;
            return node;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching band pass filter node to parent failed with error code {}", (int32_t)result);
        }

        return node;
    }

    static bool ReinitFilterNode(Internal::BandPass* node, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        
        ma_uint32 order = 2 * node->bpf.bpf2Count;
        double cutoff = node->CutoffFrequency;
        
        ma_bpf_node_config config = ma_bpf_node_config_init(node->bpf.channels, SAMPLERATE, cutoff, order);

        ma_bpf_node_uninit(node, nullptr);

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_bpf_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Reinitializing band pass filter node failed with error code {}", (int32_t)result);
        
            delete node;
            node = nullptr;
            return false;
        }

        if (!parent) {
            return true;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching band pass filter node to parent failed with error code {}", (int32_t)result);
        }
        
        return true;
    }

    template<>
    static void UninitFilterNode(Internal::BandPass* node) {
        ma_bpf_node_uninit(node, nullptr);
        delete node;
    }

    static constexpr FilterType FilterNodeToType(Internal::BandPass* node) {
        return FilterType::BandPass;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Notch Filter Node                                                                                                                 //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    static Internal::FilterNode CreateFilterNode(Effects::NotchSettings settings, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        ma_uint32 channels = ma_engine_get_channels(Internal::System->Engine);

        ma_notch_node_config config = ma_notch_node_config_init(channels, SAMPLERATE, settings.Q, settings.Frequency);

        Internal::Notch* node = new Internal::Notch;
        node->Frequency = settings.Frequency;
        node->Q = settings.Q;

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_notch_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Creating notch filter node failed with error code {}", (int32_t)result);

            delete node;
            node = nullptr;
            return node;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching notch filter node to parent failed with error code {}", (int32_t)result);
        }

        return node;
    }

    static bool ReinitFilterNode(Internal::Notch* node, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        ma_uint32 channels = ma_engine_get_channels(Internal::System->Engine);
        
        ma_notch_node_config config = ma_notch_node_config_init(channels, SAMPLERATE, node->Q, node->Frequency);
        
        ma_notch_node_uninit(node, nullptr);

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_notch_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Reinitializing notch filter node failed with error code {}", (int32_t)result);
        
            delete node;
            node = nullptr;
            return false;
        }

        if (!parent) {
            return true;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching notch filter node to parent failed with error code {}", (int32_t)result);
        }
        
        return true;
    }

    template<>
    static void UninitFilterNode(Internal::Notch* node) {
        ma_notch_node_uninit(node, nullptr);
        delete node;
    }

    static constexpr FilterType FilterNodeToType(Internal::Notch* node) {
        return FilterType::Notch;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PeakingEQ Filter Node                                                                                                             //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    static Internal::FilterNode CreateFilterNode(Effects::PeakingEQSettings settings, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        ma_uint32 channels = ma_engine_get_channels(Internal::System->Engine);

        ma_peak_node_config config = ma_peak_node_config_init(channels, SAMPLERATE, settings.GainDB, settings.Q, settings.Frequency);

        Internal::PeakingEQ* node = new Internal::PeakingEQ;
        node->GainDB = settings.GainDB;
        node->Frequency = settings.Frequency;
        node->Q = settings.Q;

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_peak_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Creating peaking EQ filter node failed with error code {}", (int32_t)result);

            delete node;
            node = nullptr;
            return node;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching peaking EQ filter node to parent failed with error code {}", (int32_t)result);
        }

        return node;
    }

    static bool ReinitFilterNode(Internal::PeakingEQ* node, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        ma_uint32 channels = ma_engine_get_channels(Internal::System->Engine);
        
        ma_peak_node_config config = ma_peak_node_config_init(channels, SAMPLERATE, node->GainDB, node->Q, node->Frequency);
        
        ma_peak_node_uninit(node, nullptr);

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_peak_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Reinitializing peaking EQ filter node failed with error code {}", (int32_t)result);
        
            delete node;
            node = nullptr;
            return true;
        }

        if (!parent) {
            return true;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching peaking EQ filter node to parent failed with error code {}", (int32_t)result);
        }
        
        return true;
    }

    template<>
    static void UninitFilterNode(Internal::PeakingEQ* node) {
        ma_peak_node_uninit(node, nullptr);
        delete node;
    }

    static constexpr FilterType FilterNodeToType(Internal::PeakingEQ* node) {
        return FilterType::PeakingEQ;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Low Shelf Filter Node                                                                                                             //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    static Internal::FilterNode CreateFilterNode(Effects::LowShelfSettings settings, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        ma_uint32 channels = ma_engine_get_channels(Internal::System->Engine);

        ma_loshelf_node_config config = ma_loshelf_node_config_init(channels, SAMPLERATE, settings.GainDB, settings.Slope, settings.Frequency);

        Internal::LowShelf* node = new Internal::LowShelf;
        node->GainDB = settings.GainDB;
        node->Slope = settings.Slope;
        node->Frequency = settings.Frequency;

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_loshelf_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Creating low shelf filter node failed with error code {}", (int32_t)result);

            delete node;
            node = nullptr;
            return node;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching low shelf filter node to parent failed with error code {}", (int32_t)result);
        }

        return node;
    }

    static bool ReinitFilterNode(Internal::LowShelf* node, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        ma_uint32 channels = ma_engine_get_channels(Internal::System->Engine);
        
        ma_loshelf_node_config config = ma_loshelf_node_config_init(channels, SAMPLERATE, node->GainDB, node->Slope, node->Frequency);

        ma_loshelf_node_uninit(node, nullptr);

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_loshelf_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Reinitializing low shelf filter node failed with error code {}", (int32_t)result);
        
            delete node;
            node = nullptr;
            return false;
        }

        if (!parent) {
            return true;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching low shelf filter node to parent failed with error code {}", (int32_t)result);
        }
        
        return true;
    }

    template<>
    static void UninitFilterNode(Internal::LowShelf* node) {
        ma_loshelf_node_uninit(node, nullptr);
        delete node;
    }

    static constexpr FilterType FilterNodeToType(Internal::LowShelf* node) {
        return  FilterType::LowShelf;
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // High Shelf Filter Node                                                                                                            //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    static Internal::FilterNode CreateFilterNode(Effects::HighShelfSettings settings, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        ma_uint32 channels = ma_engine_get_channels(Internal::System->Engine);

        ma_hishelf_node_config config = ma_hishelf_node_config_init(channels, SAMPLERATE, settings.GainDB, settings.Slope, settings.Frequency);

        Internal::HighShelf* node = new Internal::HighShelf;
        node->GainDB = settings.GainDB;
        node->Slope = settings.Slope;
        node->Frequency = settings.Frequency;

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_hishelf_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Creating high shelf filter node failed with error code {}", (int32_t)result);

            delete node;
            node = nullptr;
            return node;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching high shelf filter node to parent failed with error code {}", (int32_t)result);
        }

        return node;
    }

    static bool ReinitFilterNode(Internal::HighShelf* node, ma_node* parent) {
        RC_ASSERT(Internal::System->Engine);
        ma_uint32 channels = ma_engine_get_channels(Internal::System->Engine);
        
        ma_hishelf_node_config config = ma_hishelf_node_config_init(channels, SAMPLERATE, node->GainDB, node->Slope, node->Frequency);

        ma_hishelf_node_uninit(node, nullptr);

        ma_node_graph* graph = ma_engine_get_node_graph(Internal::System->Engine);
        ma_result result = ma_hishelf_node_init(graph, &config, nullptr, node);
        if (result != MA_SUCCESS) {
            RC_WARN("Reinitializing high shelf filter node failed with error code {}", (int32_t)result);
        
            delete node;
            node = nullptr;
            return false;
        }

        if (!parent) {
            return true;
        }

        result = ma_node_attach_output_bus(node, 0, parent, 0);
        if (result != MA_SUCCESS) {
            RC_WARN("Attaching high shelf filter node to parent failed with error code {}", (int32_t)result);
        }
        
        return true;
    }

    template<>
    static void UninitFilterNode(Internal::HighShelf* node) {
        ma_hishelf_node_uninit(node, nullptr);
        delete node;
    }

    static constexpr FilterType FilterNodeToType(Internal::HighShelf* node) {
        return FilterType::HighShelf;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Filter                                                                                                                            //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    Filter::Filter(T settings) : m_Child(nullptr), m_Parent(nullptr) {
        RC_ASSERT(Internal::System->Engine);
        ma_node* endPoint = ma_engine_get_endpoint(Internal::System->Engine);

        m_InternalFilter = CreateFilterNode(settings, endPoint);
    }

    template Filter::Filter(DelaySettings);
    template Filter::Filter(BiquadSettings);
    template Filter::Filter(LowPassSettings);
    template Filter::Filter(HighPassSettings);
    template Filter::Filter(BandPassSettings);
    template Filter::Filter(NotchSettings);
    template Filter::Filter(PeakingEQSettings);
    template Filter::Filter(HighShelfSettings);
    template Filter::Filter(LowShelfSettings);
    
    template <typename T>
    Filter::Filter(T settings, Bus& parent) : m_Child(nullptr), m_Parent(nullptr) {
        m_InternalFilter = CreateFilterNode(settings, parent.m_InternalBus.get());

        Bus* parentPtr = &parent;
        SwitchParent(parentPtr);
    }

    template Filter::Filter(DelaySettings, Bus&);
    template Filter::Filter(BiquadSettings, Bus&);
    template Filter::Filter(LowPassSettings, Bus&);
    template Filter::Filter(HighPassSettings, Bus&);
    template Filter::Filter(BandPassSettings, Bus&);
    template Filter::Filter(NotchSettings, Bus&);
    template Filter::Filter(PeakingEQSettings, Bus&);
    template Filter::Filter(HighShelfSettings, Bus&);
    template Filter::Filter(LowShelfSettings, Bus&);

    template <typename T>
    Filter::Filter(T settings, Filter& parent) : m_Child(nullptr), m_Parent(nullptr) {
        ma_node* parentNode = std::visit([](auto parent) { return (ma_node*)parent; }, parent.m_InternalFilter);
        m_InternalFilter = CreateFilterNode(settings, parentNode);

        Filter* parentPtr = &parent;
        SwitchParent(parentPtr);
    }

    template Filter::Filter(DelaySettings, Filter&);
    template Filter::Filter(BiquadSettings, Filter&);
    template Filter::Filter(LowPassSettings, Filter&);
    template Filter::Filter(HighPassSettings, Filter&);
    template Filter::Filter(BandPassSettings, Filter&);
    template Filter::Filter(NotchSettings, Filter&);
    template Filter::Filter(PeakingEQSettings, Filter&);
    template Filter::Filter(LowShelfSettings, Filter&);
    template Filter::Filter(HighShelfSettings, Filter&);

    Filter::Filter(Filter&& other) noexcept : m_Parent(nullptr), m_Child(nullptr) {
        m_InternalFilter.swap(other.m_InternalFilter);
        m_Child.swap(other.m_Child);

        SwitchParent(other.m_Parent);
        other.m_Parent = nullptr;

        // Only cases where child is another filter need to be considered
        if (std::holds_alternative<Filter*>(m_Child)) {
            std::get<Filter*>(m_Child)->SwitchParent(this);
        }
    }

    Filter::~Filter() {
        SwitchParent(nullptr);

        // Only cases where child is another filter need to be considered
        if (std::holds_alternative<Filter*>(m_Child)) {
            std::get<Filter*>(m_Child)->SwitchParent(nullptr);
        }

        std::visit([](auto node) { UninitFilterNode(node); }, m_InternalFilter);
    }

    Filter& Filter::operator=(Filter&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        std::visit([](auto& node) { UninitFilterNode(node); node = nullptr; }, m_InternalFilter);

        m_InternalFilter.swap(other.m_InternalFilter);
        m_Child.swap(other.m_Child);

        // Only cases where child is another filter need to be considered
        if (std::holds_alternative<Filter*>(m_Child)) {
            std::get<Filter*>(m_Child)->SwitchParent(this);
        }

        SwitchParent(other.m_Parent);
        other.m_Parent = nullptr;

        return *this;
    }

    bool Filter::Reinit() {
        ma_node* parentNode = nullptr;

        if (std::holds_alternative<Filter*>(m_Parent)) {
            Filter* parent = std::get<Filter*>(m_Parent);

            if (parent) {
                parentNode = std::visit([](auto parent) { return (ma_node*)parent; }, parent->m_InternalFilter);
            }
        } else if (std::holds_alternative<Bus*>(m_Parent)) {
            Bus* parent = std::get<Bus*>(m_Parent);

            if (parent) {
                parentNode = parent->m_InternalBus.get();
            }
        }

        if (!parentNode) {
            parentNode = ma_engine_get_endpoint(Internal::System->Engine);
        }

        return std::visit([parentNode, this](auto node) { return ReinitFilterNode(node, parentNode);}, m_InternalFilter);
    }
    
    FilterType Filter::GetType() {
        return std::visit([](auto node) { return FilterNodeToType(node); }, m_InternalFilter);
    }

    void Filter::AttachParent(Bus& parent) {
        if (std::holds_alternative<Bus*>(m_Parent) && std::get<Bus*>(m_Parent) == &parent) {
            return;
        }

        SwitchParent(&parent);

        ma_node* currentNode = std::visit([](auto node) { return (ma_node*)node; }, m_InternalFilter);

        // Currently implemented filter nodes and buses (ma_sound_group) have only one input and output bus, so index is always 0
        ma_result result = ma_node_attach_output_bus(currentNode, 0, parent.m_InternalBus.get(), 0);

        if (result != MA_SUCCESS) {
            RC_WARN("Attaching parent bus to filter failed with error {}", (int32_t)result);
        }
    }

    void Filter::AttachParent(Filter& parent) {
        if (std::holds_alternative<Filter*>(m_Parent) && std::get<Filter*>(m_Parent) == &parent) {
            return;
        }

        SwitchParent(&parent);

        ma_node* currentNode = std::visit([](auto node) { return (ma_node*)node; }, m_InternalFilter);
        ma_node* parentNode = std::visit([](auto parent) { return (ma_node*)parent; }, parent.m_InternalFilter);

        // Currently implemented filter nodes have only one input and output bus, so index is always 0
        ma_result result = ma_node_attach_output_bus(currentNode, 0, parentNode, 0);

        if (result != MA_SUCCESS) {
            RC_WARN("Attaching parent filter to filter failed with error {}", (int32_t)result);
        }
    }

    void Filter::DetachParent() {
        SwitchParent(nullptr);

        ma_node* currentNode = std::visit([](auto node) { return (ma_node*)node; }, m_InternalFilter);
        ma_node* endPoint = ma_engine_get_endpoint(Internal::System->Engine);

        // Currently implemented filter nodes have only one input and output bus, so index is always 0
        ma_result result = ma_node_attach_output_bus(currentNode, 0, endPoint, 0);

        if (result != MA_SUCCESS) {
            RC_WARN("Attaching engine as filter parent failed with error {}", (int32_t)result);
        }
    }

    void Filter::SwitchParent(RelativeNode parent) {
        // Only cases where parent is another filter need to be considered
        if (std::holds_alternative<Filter*>(m_Parent)) {
            std::get<Filter*>(m_Parent)->DetachChild();
        }

        // Only cases where parent is another filter need to be considered
        if (std::holds_alternative<Filter*>(parent)) {
            Filter* newParent = std::get<Filter*>(parent);
          
            if (newParent == this) {
                m_Parent = nullptr;
                return;
            }
            
            newParent->AttachChild(this);
        }

        m_Parent = parent;
    }

    void Filter::AttachChild(RelativeNode child) {
        if (std::holds_alternative<Filter*>(child) && std::get<Filter*>(child) == this) {
            return;
        }

        m_Child = child;
    }

    void Filter::DetachChild() {
        m_Child = nullptr;
    }
}