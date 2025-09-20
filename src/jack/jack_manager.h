#pragma once

#include "utils.h"

#include <string>
#include <vector>
#include <mutex>
#include <jack/jack.h>

class JackManager {
public:
    static JackManager& instance();

    bool init();
    void close();

    JackData* get_jack_data(bool withTransport);
    unsigned get_jack_buffer_size() const;
    unsigned set_jack_buffer_size(unsigned size);
    float get_jack_sample_rate() const;
    const char* get_jack_port_alias(const char* portname) const;
    const char* const* get_jack_hardware_ports(const bool isAudio, bool isOutput);

    bool has_midi_beat_clock_sender_port() const;
    bool has_serial_midi_input_port() const;
    bool has_serial_midi_output_port() const;
    bool has_midi_merger_output_port() const;
    bool has_midi_broadcaster_input_port() const;
    bool has_duox_split_spdif() const;

    bool connect_jack_ports(const char* port1, const char* port2);
    bool connect_jack_midi_output_ports(const char* port);
    bool disconnect_jack_ports(const char* port1, const char* port2);
    bool disconnect_all_jack_ports(const char* port);

    void reset_xruns();

    void init_bypass();
    bool get_truebypass_value(bool right) const;
    bool set_truebypass_value(bool right, bool bypassed);
    float get_master_volume(bool right) const;

    void set_util_callbacks(JackBufSizeChanged bufSizeChanged,
                            JackPortAppeared portAppeared,
                            JackPortDeleted portDeleted,
                            TrueBypassStateChanged trueBypassChanged);

    void set_extra_util_callbacks(CvExpInputModeChanged cvExpInputModeChanged);

private:
    JackManager();
    ~JackManager();

    // Non-copyable
    JackManager(const JackManager&) = delete;
    JackManager& operator=(const JackManager&) = delete;

    struct Impl;
    Impl* impl_;

    // Internal JACK callbacks (implemented in .cpp)
    static int InternalJackBufSize(jack_nframes_t frames, void*);
    static void InternalJackPortRegistration(jack_port_id_t port_id, int reg, void*);
    static int InternalJackXRun(void*);
    static void InternalJackShutdown(void*);
};
