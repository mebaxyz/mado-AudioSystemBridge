// SPDX-FileCopyrightText: 2012-2023 MOD Audio UG
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "utils.h"

#include <cstdio>
#include <cstring>

#ifdef HAVE_ALSA
#include <alsa/asoundlib.h>
#endif
#include <jack/jack.h>

#include <algorithm>
#include <mutex>
#include <string>
#include <vector>

#define ALSA_SOUNDCARD_DEFAULT_ID  "MODDUO"
#define ALSA_CONTROL_BYPASS_LEFT   "Left True-Bypass"
#define ALSA_CONTROL_BYPASS_RIGHT  "Right True-Bypass"
#define ALSA_CONTROL_LOOPBACK1     "LOOPBACK"
#define ALSA_CONTROL_LOOPBACK2     "Loopback Switch"
#define ALSA_CONTROL_SPDIF_ENABLE  "SPDIF Enable"
#define ALSA_CONTROL_MASTER_VOLUME "DAC"

#define JACK_EXTERNAL_PREFIX     "mod-external"
#define JACK_EXTERNAL_PREFIX_LEN 12

// --------------------------------------------------------------------------------------------------------
// Delegate implementation to JackManager to allow incremental refactor.
#include "jack/jack_manager.h"

// ...existing code...

// The low-level JACK/ALSA callbacks and helpers are implemented in
// `jack/jack_manager.cpp` (JackManager). Keeping them here would
// duplicate symbols and global state; callers use the delegating
// wrappers below (which call JackManager::instance()).

bool init_jack(void)
{
    return JackManager::instance().init();
}

void close_jack(void)
{
    JackManager::instance().close();
}

// --------------------------------------------------------------------------------------------------------

JackData* get_jack_data(bool withTransport)
{
    return JackManager::instance().get_jack_data(withTransport);
}

unsigned get_jack_buffer_size(void)
{
    return JackManager::instance().get_jack_buffer_size();
}

unsigned set_jack_buffer_size(unsigned size)
{
    return JackManager::instance().set_jack_buffer_size(size);
}

float get_jack_sample_rate(void)
{
    return JackManager::instance().get_jack_sample_rate();
}

const char* get_jack_port_alias(const char* portname)
{
    return JackManager::instance().get_jack_port_alias(portname);
}

const char* const* get_jack_hardware_ports(const bool isAudio, bool isOutput)
{
    return JackManager::instance().get_jack_hardware_ports(isAudio, isOutput);
}

// --------------------------------------------------------------------------------------------------------

bool has_midi_beat_clock_sender_port(void)
{
    return JackManager::instance().has_midi_beat_clock_sender_port();
}

bool has_serial_midi_input_port(void)
{
    return JackManager::instance().has_serial_midi_input_port();
}

bool has_serial_midi_output_port(void)
{
    return JackManager::instance().has_serial_midi_output_port();
}

/**
 * Ask the JACK server if there is the midi-merger client available.
 */
bool has_midi_merger_output_port(void)
{
    return JackManager::instance().has_midi_merger_output_port();
}

/**
 * Ask the JACK server if there is the midi-broadcaster client available.
 */
bool has_midi_broadcaster_input_port(void)
{
    return JackManager::instance().has_midi_broadcaster_input_port();
}

bool has_duox_split_spdif(void)
{
    return JackManager::instance().has_duox_split_spdif();
}

// --------------------------------------------------------------------------------------------------------

bool connect_jack_ports(const char* port1, const char* port2)
{
    return JackManager::instance().connect_jack_ports(port1, port2);
}

bool connect_jack_midi_output_ports(const char* port)
{
    return JackManager::instance().connect_jack_midi_output_ports(port);
}

bool disconnect_jack_ports(const char* port1, const char* port2)
{
    return JackManager::instance().disconnect_jack_ports(port1, port2);
}

bool disconnect_all_jack_ports(const char* portname)
{
    return JackManager::instance().disconnect_all_jack_ports(portname);
}

void reset_xruns(void)
{
    JackManager::instance().reset_xruns();
}

// --------------------------------------------------------------------------------------------------------

void init_bypass(void)
{
    JackManager::instance().init_bypass();
}

bool get_truebypass_value(bool right)
{
    return JackManager::instance().get_truebypass_value(right);
}

bool set_truebypass_value(bool right, bool bypassed)
{
    return JackManager::instance().set_truebypass_value(right, bypassed);
}

float get_master_volume(bool right)
{
    return JackManager::instance().get_master_volume(right);
}

// --------------------------------------------------------------------------------------------------------

void set_util_callbacks(JackBufSizeChanged bufSizeChanged,
                        JackPortAppeared portAppeared,
                        JackPortDeleted portDeleted,
                        TrueBypassStateChanged trueBypassChanged)
{
    JackManager::instance().set_util_callbacks(bufSizeChanged, portAppeared, portDeleted, trueBypassChanged);
}


void set_extra_util_callbacks(CvExpInputModeChanged cvExpInputModeChanged)
{
    JackManager::instance().set_extra_util_callbacks(cvExpInputModeChanged);
}

// --------------------------------------------------------------------------------------------------------
