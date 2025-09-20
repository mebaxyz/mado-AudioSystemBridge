#include "jack_manager.h"

#include <jack/jack.h>
#ifdef HAVE_ALSA
#include <alsa/asoundlib.h>
#endif

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>

#define ALSA_SOUNDCARD_DEFAULT_ID  "MODDUO"
#define ALSA_CONTROL_BYPASS_LEFT   "Left True-Bypass"
#define ALSA_CONTROL_BYPASS_RIGHT  "Right True-Bypass"
#define ALSA_CONTROL_LOOPBACK1     "LOOPBACK"
#define ALSA_CONTROL_LOOPBACK2     "Loopback Switch"
#define ALSA_CONTROL_SPDIF_ENABLE  "SPDIF Enable"
#define ALSA_CONTROL_MASTER_VOLUME "DAC"

#define JACK_EXTERNAL_PREFIX     "mod-external"
#define JACK_EXTERNAL_PREFIX_LEN 12

#include <algorithm>
#include <mutex>
#include <string>
#include <vector>

// Internal implementation mirroring previous globals from utils_jack.cpp
struct JackManager::Impl {
    jack_client_t* gClient = nullptr;
    volatile unsigned gNewBufSize = 0;
    volatile unsigned gXrunCount = 0;
    const char** gPortListRet = nullptr;

    std::mutex gPortRegisterMutex;
    std::vector<std::string> gRegisteredPorts;

    std::mutex gPortUnregisterMutex;
    std::vector<std::string> gUnregisteredPorts;

#ifdef HAVE_ALSA
    snd_mixer_t* gAlsaMixer = nullptr;
    snd_mixer_elem_t* gAlsaControlLeft = nullptr;
    snd_mixer_elem_t* gAlsaControlRight = nullptr;
    snd_mixer_elem_t* gAlsaControlCvExp = nullptr;
    bool gLastAlsaValueLeft = true;
    bool gLastAlsaValueRight = true;
    bool gLastAlsaValueCvExp = false;
#endif

    JackBufSizeChanged jack_bufsize_changed_cb = nullptr;
    JackPortAppeared jack_port_appeared_cb = nullptr;
    JackPortDeleted jack_port_deleted_cb = nullptr;
    TrueBypassStateChanged true_bypass_changed_cb = nullptr;
#ifdef HAVE_ALSA
    CvExpInputModeChanged cv_exp_mode_changed_cb = nullptr;
#endif
};

JackManager& JackManager::instance() {
    static JackManager inst;
    return inst;
}

JackManager::JackManager() : impl_(new Impl()) {}
JackManager::~JackManager() { delete impl_; }

bool JackManager::init() {
    // Ported from utils_jack.cpp::init_jack
#ifdef HAVE_ALSA
    if (impl_->gAlsaMixer == nullptr)
    {
        if (snd_mixer_open(&impl_->gAlsaMixer, SND_MIXER_ELEM_SIMPLE) == 0)
        {
            snd_mixer_selem_id_t* sid;

            char soundcard[32] = "hw:";

            if (const char* const cardname = getenv("MOD_SOUNDCARD"))
                strncat(soundcard, cardname, 28);
            else
                strncat(soundcard, ALSA_SOUNDCARD_DEFAULT_ID, 28);

            soundcard[31] = '\0';

            if (snd_mixer_attach(impl_->gAlsaMixer, soundcard) == 0 &&
                snd_mixer_selem_register(impl_->gAlsaMixer, nullptr, nullptr) == 0 &&
                snd_mixer_load(impl_->gAlsaMixer) == 0 &&
                snd_mixer_selem_id_malloc(&sid) == 0)
            {
                snd_mixer_selem_id_set_index(sid, 0);
                snd_mixer_selem_id_set_name(sid, ALSA_CONTROL_BYPASS_LEFT);
                impl_->gAlsaControlLeft = snd_mixer_find_selem(impl_->gAlsaMixer, sid);

                snd_mixer_selem_id_set_index(sid, 0);
                snd_mixer_selem_id_set_name(sid, ALSA_CONTROL_BYPASS_RIGHT);
                impl_->gAlsaControlRight = snd_mixer_find_selem(impl_->gAlsaMixer, sid);

#ifdef _MOD_DEVICE_DUOX
                snd_mixer_selem_id_set_index(sid, 0);
                snd_mixer_selem_id_set_name(sid, "CV/Exp.Pedal Mode");
                impl_->gAlsaControlCvExp = snd_mixer_find_selem(impl_->gAlsaMixer, sid);
#endif

                snd_mixer_selem_id_free(sid);
            }
            else
            {
                snd_mixer_close(impl_->gAlsaMixer);
                impl_->gAlsaMixer = nullptr;
            }
        }
    }
#endif

    if (impl_->gClient != nullptr)
    {
        printf("jack client activated before, nothing to do\n");
        return true;
    }

#ifdef _MOD_DESKTOP
    const jack_options_t options = static_cast<jack_options_t>(JackNoStartServer|JackUseExactName|JackServerName);
    const char* servername = std::getenv("MOD_DESKTOP_SERVER_NAME");
    if (servername == nullptr)
        servername = "mod-desktop";
    jack_client_t* const client = jack_client_open("mod-ui", options, nullptr, servername);
#else
    const jack_options_t options = static_cast<jack_options_t>(JackNoStartServer|JackUseExactName);
    jack_client_t* const client = jack_client_open("mod-ui", options, nullptr);
#endif

    if (client == nullptr)
        return false;

    jack_set_buffer_size_callback(client, JackManager::InternalJackBufSize, nullptr);
    jack_set_port_registration_callback(client, JackManager::InternalJackPortRegistration, nullptr);
    jack_set_xrun_callback(client, JackManager::InternalJackXRun, nullptr);
    jack_on_shutdown(client, JackManager::InternalJackShutdown, nullptr);

    impl_->gClient = client;
    impl_->gNewBufSize = 0;
    impl_->gXrunCount = 0;
    jack_activate(client);

    printf("jack client activated\n");
    return true;
}

void JackManager::close() {
    // Ported from utils_jack.cpp::close_jack
    if (impl_->gPortListRet != nullptr)
    {
        jack_free(const_cast<char**>(impl_->gPortListRet));
        impl_->gPortListRet = nullptr;
    }

#ifdef HAVE_ALSA
    if (impl_->gAlsaMixer != nullptr)
    {
        impl_->gAlsaControlLeft = nullptr;
        impl_->gAlsaControlRight = nullptr;
        snd_mixer_close(impl_->gAlsaMixer);
        impl_->gAlsaMixer = nullptr;
    }
#endif

    if (impl_->gClient == nullptr)
    {
        printf("jack client deactivated NOT\n");
        return;
    }

    jack_client_t* const client = impl_->gClient;
    impl_->gClient = nullptr;

    jack_deactivate(client);
    jack_client_close(client);

    printf("jack client deactivated\n");
}

JackData* JackManager::get_jack_data(bool withTransport) { return nullptr; }
unsigned JackManager::get_jack_buffer_size() const { return 0; }
unsigned JackManager::set_jack_buffer_size(unsigned size) { return 0; }
float JackManager::get_jack_sample_rate() const { return 48000.0f; }
const char* JackManager::get_jack_port_alias(const char* portname) const { return nullptr; }
const char* const* JackManager::get_jack_hardware_ports(const bool isAudio, bool isOutput) { return nullptr; }

bool JackManager::has_midi_beat_clock_sender_port() const { return false; }
bool JackManager::has_serial_midi_input_port() const { return false; }
bool JackManager::has_serial_midi_output_port() const { return false; }
bool JackManager::has_midi_merger_output_port() const { return false; }
bool JackManager::has_midi_broadcaster_input_port() const { return false; }
bool JackManager::has_duox_split_spdif() const { return false; }

bool JackManager::connect_jack_ports(const char* port1, const char* port2) { return false; }
bool JackManager::connect_jack_midi_output_ports(const char* port) { return false; }
bool JackManager::disconnect_jack_ports(const char* port1, const char* port2) { return false; }
bool JackManager::disconnect_all_jack_ports(const char* port) { return false; }

void JackManager::reset_xruns() {}

void JackManager::init_bypass() {}
bool JackManager::get_truebypass_value(bool right) const { return false; }
bool JackManager::set_truebypass_value(bool right, bool bypassed) { return false; }
float JackManager::get_master_volume(bool right) const { return -127.5f; }

void JackManager::set_util_callbacks(JackBufSizeChanged bufSizeChanged,
                                    JackPortAppeared portAppeared,
                                    JackPortDeleted portDeleted,
                                    TrueBypassStateChanged trueBypassChanged) {}

void JackManager::set_extra_util_callbacks(CvExpInputModeChanged cvExpInputModeChanged) {}

// -----------------------------------------------------------------------------
// Now implement the previously stubbed methods with content ported from utils_jack.cpp

static bool _get_alsa_switch_value(snd_mixer_elem_t* const elem)
{
    int val = 0;
    snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_MONO, &val);
    return (val != 0);
}

int JackManager::InternalJackBufSize(jack_nframes_t frames, void*)
{
    instance().impl_->gNewBufSize = frames;
    return 0;
}

void JackManager::InternalJackPortRegistration(jack_port_id_t port_id, int reg, void*)
{
    JackManager& mgr = instance();
    Impl* d = mgr.impl_;

    if (d->gClient == nullptr)
        return;

    if (reg)
    {
        if (d->jack_port_appeared_cb == nullptr)
            return;
    }
    else
    {
        if (d->jack_port_deleted_cb == nullptr)
            return;
    }

    if (const jack_port_t* const port = jack_port_by_id(d->gClient, port_id))
    {
        if ((jack_port_flags(port) & JackPortIsPhysical) == 0x0 && reg != 0)
            return;

        if (const char* const port_name = jack_port_name(port))
        {
            if (strncmp(port_name, "system:midi_", 12) != 0 &&
                strncmp(port_name, JACK_EXTERNAL_PREFIX ":", JACK_EXTERNAL_PREFIX_LEN + 1) != 0 &&
                strncmp(port_name, "nooice", 5) != 0)
                return;

            const std::string portName(port_name);

            if (reg)
            {
                const std::lock_guard<std::mutex> clg(d->gPortRegisterMutex);

                if (std::find(d->gRegisteredPorts.begin(), d->gRegisteredPorts.end(), portName) == d->gRegisteredPorts.end())
                    d->gRegisteredPorts.push_back(portName);
            }
            else
            {
                const std::lock_guard<std::mutex> clgr(d->gPortRegisterMutex);
                const std::lock_guard<std::mutex> clgu(d->gPortUnregisterMutex);

                if (std::find(d->gUnregisteredPorts.begin(), d->gUnregisteredPorts.end(), portName) == d->gUnregisteredPorts.end())
                    d->gUnregisteredPorts.push_back(portName);

                const std::vector<std::string>::iterator portNameItr =
                    std::find(d->gRegisteredPorts.begin(), d->gRegisteredPorts.end(), portName);
                if (portNameItr != d->gRegisteredPorts.end())
                    d->gRegisteredPorts.erase(portNameItr);
            }
        }
    }
}

int JackManager::InternalJackXRun(void*)
{
    instance().impl_->gXrunCount += 1;
    return 0;
}

void JackManager::InternalJackShutdown(void*)
{
    instance().impl_->gClient = nullptr;
}

JackData* JackManager::get_jack_data(bool withTransport)
{
    static JackData data = { 0.0f, 0, false, 4.0, 120.0 };
    static std::vector<std::string> localPorts;

    Impl* d = impl_;

    if (d->gClient != nullptr)
    {
        data.cpuLoad = jack_cpu_load(d->gClient);
        data.xruns = d->gXrunCount;

        if (withTransport)
        {
            jack_position_t pos;
            data.rolling = jack_transport_query(d->gClient, &pos) == JackTransportRolling;

            if (pos.valid & JackTransportBBT)
            {
                data.bpb = pos.beats_per_bar;
                data.bpm = pos.beats_per_minute;
            }
            else
            {
                data.bpb = 4.0;
                data.bpm = 120.0;
            }
        }

        if (d->jack_port_deleted_cb != nullptr)
        {
            {
                const std::lock_guard<std::mutex> clg(d->gPortUnregisterMutex);

                if (d->gUnregisteredPorts.size() > 0)
                    d->gUnregisteredPorts.swap(localPorts);
            }

            for (const std::string& portName : localPorts)
                d->jack_port_deleted_cb(portName.c_str());

            localPorts.clear();
        }

        if (d->jack_port_appeared_cb != nullptr)
        {
            {
                const std::lock_guard<std::mutex> clg(d->gPortRegisterMutex);

                if (d->gRegisteredPorts.size() > 0)
                    d->gRegisteredPorts.swap(localPorts);
            }

            for (const std::string& portName : localPorts)
            {
                if (jack_port_t* const port = jack_port_by_name(d->gClient, portName.c_str()))
                {
                    const bool isOutput = jack_port_flags(port) & JackPortIsInput; // inverted on purpose
                    d->jack_port_appeared_cb(portName.c_str(), isOutput);
                }
            }

            localPorts.clear();
        }
    }
    else
    {
        data.cpuLoad = 0.0f;
        data.xruns   = 0;
        data.rolling = false;
        data.bpb     = 4.0f;
        data.bpm     = 120.0f;
    }

    if (d->gNewBufSize > 0 && d->jack_bufsize_changed_cb != nullptr)
    {
        const unsigned int bufsize = d->gNewBufSize;
        d->gNewBufSize = 0;

        d->jack_bufsize_changed_cb(bufsize);
    }

#ifdef HAVE_ALSA
    if (d->gAlsaMixer != nullptr && (d->true_bypass_changed_cb != nullptr || d->cv_exp_mode_changed_cb != nullptr))
    {
        bool changedBypass = false;
        snd_mixer_handle_events(d->gAlsaMixer);

        if (d->gAlsaControlLeft != nullptr)
        {
            const bool newValue = _get_alsa_switch_value(d->gAlsaControlLeft);

            if (d->gLastAlsaValueLeft != newValue)
            {
                changedBypass = true;
                d->gLastAlsaValueLeft = newValue;
            }
        }

        if (d->gAlsaControlRight != nullptr)
        {
            const bool newValue = _get_alsa_switch_value(d->gAlsaControlRight);

            if (d->gLastAlsaValueRight != newValue)
            {
                changedBypass = true;
                d->gLastAlsaValueRight = newValue;
            }
        }

        if (changedBypass && d->true_bypass_changed_cb != nullptr)
            d->true_bypass_changed_cb(d->gLastAlsaValueLeft, d->gLastAlsaValueRight);

#ifdef _MOD_DEVICE_DUOX
        bool changedCvExpMode = false;

        if (d->gAlsaControlCvExp != nullptr)
        {
            bool newValue = d->gLastAlsaValueCvExp;

            snd_mixer_t* mixer;
            if (snd_mixer_open(&mixer, SND_MIXER_ELEM_SIMPLE) == 0)
            {
                snd_mixer_selem_id_t* sid;
                if (snd_mixer_attach(mixer, "hw:DUOX") == 0 &&
                    snd_mixer_selem_register(mixer, nullptr, nullptr) == 0 &&
                    snd_mixer_load(mixer) == 0 &&
                    snd_mixer_selem_id_malloc(&sid) == 0)
                {
                    snd_mixer_selem_id_set_index(sid, 0);
                    snd_mixer_selem_id_set_name(sid, "CV/Exp.Pedal Mode");

                    if (snd_mixer_elem_t* const elem = snd_mixer_find_selem(mixer, sid))
                        newValue = _get_alsa_switch_value(elem);

                    snd_mixer_selem_id_free(sid);
                }

                snd_mixer_close(mixer);
            }

            if (d->gLastAlsaValueCvExp != newValue)
            {
                changedCvExpMode = true;
                d->gLastAlsaValueCvExp = newValue;
            }
        }

        if (changedCvExpMode && d->cv_exp_mode_changed_cb != nullptr)
            d->cv_exp_mode_changed_cb(d->gLastAlsaValueCvExp);
#endif
    }
#endif

    return &data;
}

unsigned JackManager::get_jack_buffer_size() const
{
    if (impl_->gClient == nullptr)
        return 128;

    return jack_get_buffer_size(impl_->gClient);
}

unsigned JackManager::set_jack_buffer_size(unsigned size)
{
    if (impl_->gClient == nullptr)
        return 0;

    if (jack_set_buffer_size(impl_->gClient, size) == 0)
        return size;

    return jack_get_buffer_size(impl_->gClient);
}

float JackManager::get_jack_sample_rate() const
{
    if (impl_->gClient == nullptr)
        return 48000.0f;

    return jack_get_sample_rate(impl_->gClient);
}

const char* JackManager::get_jack_port_alias(const char* portname) const
{
    static char  aliases[2][320];
    static char* aliasesptr[2] = {
        aliases[0],
        aliases[1]
    };

    if (impl_->gClient != nullptr)
        if (jack_port_t* const port = jack_port_by_name(impl_->gClient, portname))
            if (jack_port_get_aliases(port, aliasesptr) > 0)
                return aliases[0];

    return nullptr;
}

const char* const* JackManager::get_jack_hardware_ports(const bool isAudio, bool isOutput)
{
    if (impl_->gPortListRet != nullptr)
    {
        jack_free(const_cast<char**>(impl_->gPortListRet));
        impl_->gPortListRet = nullptr;
    }

    if (impl_->gClient == nullptr)
        return nullptr;

    const unsigned long flags = JackPortIsPhysical | (isOutput ? JackPortIsInput : JackPortIsOutput);
    const char* const type    = isAudio ? JACK_DEFAULT_AUDIO_TYPE : JACK_DEFAULT_MIDI_TYPE;
    const char** const ports  = jack_get_ports(impl_->gClient, "", type, flags);

    if (ports == nullptr)
        return nullptr;

    // hide midi-through capture ports
    if (!isAudio && !isOutput)
    {
        static char  aliases[2][320];
        static char* aliasesptr[2] = {
            aliases[0],
            aliases[1]
        };

        for (int i=0; ports[i] != nullptr; ++i)
        {
            if (strncmp(ports[i], "system:midi_capture_", 20))
                continue;

            jack_port_t* const port = jack_port_by_name(impl_->gClient, ports[i]);

            if (port == nullptr)
                continue;
            if (jack_port_get_aliases(port, aliasesptr) <= 0)
                continue;
            if (strncmp(aliases[0], "alsa_pcm:Midi-Through/", 22))
                continue;

            for (int j=i; ports[j] != nullptr; ++j)
                ports[j] = ports[j+1];
            --i;
        }
    }

#ifdef _MOD_DEVICE_DUOX
    // Duo X special case for SPDIF mirrored mode
    if (isAudio && isOutput && ! has_duox_split_spdif())
    {
        for (int i=0; ports[i] != nullptr; ++i)
        {
            if (ports[i+1] == nullptr)
                break;
            if (std::strcmp(ports[i], "system:playback_3") != 0)
                continue;
            if (std::strcmp(ports[i+1], "system:playback_4") != 0)
                continue;

            for (int j=i+2; ports[j] != nullptr; ++i, ++j)
                ports[i] = ports[j];

            ports[i] = nullptr;
            break;
        }
    }
#endif

    impl_->gPortListRet = ports;

    return ports;
}

bool JackManager::has_midi_beat_clock_sender_port() const
{
    if (impl_->gClient == nullptr)
        return false;

    return (jack_port_by_name(impl_->gClient, "effect_9993:mclk") != nullptr);
}

bool JackManager::has_serial_midi_input_port() const
{
    if (impl_->gClient == nullptr)
        return false;

    return (jack_port_by_name(impl_->gClient, "ttymidi:MIDI_in") != nullptr);
}

bool JackManager::has_serial_midi_output_port() const
{
    if (impl_->gClient == nullptr)
        return false;

    return (jack_port_by_name(impl_->gClient, "ttymidi:MIDI_out") != nullptr);
}

bool JackManager::has_midi_merger_output_port() const
{
    if (impl_->gClient == nullptr)
        return false;

    return (jack_port_by_name(impl_->gClient, "mod-midi-merger:out") != nullptr);
}

bool JackManager::has_midi_broadcaster_input_port() const
{
    if (impl_->gClient == nullptr)
        return false;

    return (jack_port_by_name(impl_->gClient, "mod-midi-broadcaster:in") != nullptr);
}

bool JackManager::has_duox_split_spdif() const
{
#ifdef _MOD_DEVICE_DUOX
    if (impl_->gClient == nullptr)
        return false;

    return (jack_port_by_name(impl_->gClient, "mod-monitor:in_3") != nullptr);
#else
    return false;
#endif
}

bool JackManager::connect_jack_ports(const char* port1, const char* port2)
{
    if (impl_->gClient == nullptr)
        return false;

    int ret;

    ret = jack_connect(impl_->gClient, port1, port2);
    if (ret == 0 || ret == EEXIST)
        return true;

    ret = jack_connect(impl_->gClient, port2, port1);
    if (ret == 0 || ret == EEXIST)
        return true;

    return false;
}

bool JackManager::connect_jack_midi_output_ports(const char* port)
{
    if (impl_->gClient == nullptr)
        return false;

    int ret;

    if (jack_port_by_name(impl_->gClient, "mod-midi-broadcaster:in") != nullptr)
    {
        ret = jack_connect(impl_->gClient, port, "mod-midi-broadcaster:in");
        return (ret == 0 || ret == EEXIST);
    }

    if (const char** const ports = jack_get_ports(impl_->gClient, "",
                                                  JACK_DEFAULT_MIDI_TYPE,
                                                  JackPortIsPhysical | JackPortIsInput))
    {
        for (int i=0; ports[i] != nullptr; ++i)
            jack_connect(impl_->gClient, port, ports[i]);

        jack_free(ports);
        return true;
    }

    return false;
}

bool JackManager::disconnect_jack_ports(const char* port1, const char* port2)
{
    if (impl_->gClient == nullptr)
        return false;

    if (jack_disconnect(impl_->gClient, port1, port2) == 0)
        return true;
    if (jack_disconnect(impl_->gClient, port2, port1) == 0)
        return true;

    return false;
}

bool JackManager::disconnect_all_jack_ports(const char* portname)
{
    if (impl_->gClient == nullptr)
        return false;

    jack_port_t* const port = jack_port_by_name(impl_->gClient, portname);

    if (port == nullptr)
        return false;

    const bool isOutput = jack_port_flags(port) & JackPortIsOutput;

    if (const char** const ports = jack_port_get_all_connections(impl_->gClient, port))
    {
        for (int i=0; ports[i] != nullptr; ++i)
        {
            if (isOutput)
                jack_disconnect(impl_->gClient, portname, ports[i]);
            else
                jack_disconnect(impl_->gClient, ports[i], portname);
        }

        jack_free(ports);
    }

    return true;
}

void JackManager::reset_xruns()
{
    impl_->gXrunCount = 0;
}

void JackManager::init_bypass()
{
#ifdef HAVE_ALSA
    if (impl_->gAlsaMixer == nullptr)
        return;

    if (impl_->gAlsaControlLeft != nullptr)
        snd_mixer_selem_set_playback_switch_all(impl_->gAlsaControlLeft, 0);

    if (impl_->gAlsaControlRight != nullptr)
        snd_mixer_selem_set_playback_switch_all(impl_->gAlsaControlRight, 0);

#ifdef _MOD_DEVICE_DUOX
    if (impl_->gAlsaControlCvExp != nullptr)
        impl_->gLastAlsaValueCvExp = _get_alsa_switch_value(impl_->gAlsaControlCvExp);
#endif

    snd_mixer_selem_id_t* sid;
    if (snd_mixer_selem_id_malloc(&sid) == 0)
    {
        snd_mixer_selem_id_set_index(sid, 0);
        snd_mixer_selem_id_set_name(sid, ALSA_CONTROL_LOOPBACK1);

        if (snd_mixer_elem_t* const elem = snd_mixer_find_selem(impl_->gAlsaMixer, sid))
            snd_mixer_selem_set_playback_switch_all(elem, 0);

        snd_mixer_selem_id_set_index(sid, 0);
        snd_mixer_selem_id_set_name(sid, ALSA_CONTROL_LOOPBACK2);

        if (snd_mixer_elem_t* const elem = snd_mixer_find_selem(impl_->gAlsaMixer, sid))
            snd_mixer_selem_set_playback_switch_all(elem, 0);

        snd_mixer_selem_id_set_index(sid, 0);
        snd_mixer_selem_id_set_name(sid, ALSA_CONTROL_SPDIF_ENABLE);

        if (snd_mixer_elem_t* const elem = snd_mixer_find_selem(impl_->gAlsaMixer, sid))
            snd_mixer_selem_set_playback_switch_all(elem, 1);

        snd_mixer_selem_id_free(sid);
    }
#endif
}

bool JackManager::get_truebypass_value(bool right) const
{
#ifdef HAVE_ALSA
    return right ? impl_->gLastAlsaValueRight : impl_->gLastAlsaValueLeft;
#else
    return false;

    (void)right;
#endif
}

bool JackManager::set_truebypass_value(bool right, bool bypassed)
{
#ifdef HAVE_ALSA
    if (impl_->gAlsaMixer == nullptr)
        return false;

    if (right)
    {
        if (impl_->gAlsaControlRight != nullptr)
            return (snd_mixer_selem_set_playback_switch_all(impl_->gAlsaControlRight, bypassed) == 0);
    }
    else
    {
        if (impl_->gAlsaControlLeft != nullptr)
            return (snd_mixer_selem_set_playback_switch_all(impl_->gAlsaControlLeft, bypassed) == 0);
    }
#endif

    return false;

#ifndef HAVE_ALSA
    (void)right;
    (void)bypassed;
#endif
}

float JackManager::get_master_volume(bool right) const
{
#ifdef HAVE_ALSA
    if (impl_->gAlsaMixer == nullptr)
        return -127.5f;

    snd_mixer_selem_id_t* sid;

    if (snd_mixer_selem_id_malloc(&sid) != 0)
        return -127.5f;

    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, ALSA_CONTROL_MASTER_VOLUME);

    float val = -127.5f;

    if (snd_mixer_elem_t* const elem = snd_mixer_find_selem(impl_->gAlsaMixer, sid))
    {
        long aval = 0;
        snd_mixer_selem_get_playback_volume(elem,
                                            right ? SND_MIXER_SCHN_FRONT_RIGHT : SND_MIXER_SCHN_FRONT_LEFT,
                                            &aval);

        const float a = 127.5f / 255.f;
        const float b = - a * 255.f;
        val = a * aval + b;
    }

    snd_mixer_selem_id_free(sid);
    return val;
#else
    return -127.5f;

    (void)right;
#endif
}

void JackManager::set_util_callbacks(JackBufSizeChanged bufSizeChanged,
                                    JackPortAppeared portAppeared,
                                    JackPortDeleted portDeleted,
                                    TrueBypassStateChanged trueBypassChanged)
{
    impl_->jack_bufsize_changed_cb = bufSizeChanged;
    impl_->jack_port_appeared_cb   = portAppeared;
    impl_->jack_port_deleted_cb    = portDeleted;
    impl_->true_bypass_changed_cb  = trueBypassChanged;
}

void JackManager::set_extra_util_callbacks(CvExpInputModeChanged cvExpInputModeChanged)
{
#ifdef _MOD_DEVICE_DUOX
    impl_->cv_exp_mode_changed_cb = cvExpInputModeChanged;
#else
    (void)cvExpInputModeChanged;
#ifdef HAVE_ALSA
    (void)impl_->gAlsaControlCvExp;
    (void)impl_->gLastAlsaValueCvExp;
#endif
#endif
}
