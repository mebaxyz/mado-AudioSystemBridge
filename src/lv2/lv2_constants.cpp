#include "lv2_constants.h"

// Minimal definitions. These are kept as const char* arrays so they can be used in
// code that expects C strings coming from the legacy utils files.

const char* const kCategoryDelayPlugin[] = { "http://example.org/categories/delay", nullptr };
const char* const kCategoryDistortionPlugin[] = { "http://example.org/categories/distortion", nullptr };
const char* const kCategoryWaveshaperPlugin[] = { "http://example.org/categories/waveshaper", nullptr };
const char* const kCategoryDynamicsPlugin[] = { "http://example.org/categories/dynamics", nullptr };
const char* const kCategoryAmplifierPlugin[] = { "http://example.org/categories/amplifier", nullptr };
const char* const kCategoryCompressorPlugin[] = { "http://example.org/categories/compressor", nullptr };
const char* const kCategoryExpanderPlugin[] = { "http://example.org/categories/expander", nullptr };
const char* const kCategoryGatePlugin[] = { "http://example.org/categories/gate", nullptr };
const char* const kCategoryLimiterPlugin[] = { "http://example.org/categories/limiter", nullptr };
const char* const kCategoryFilterPlugin[] = { "http://example.org/categories/filter", nullptr };
const char* const kCategoryAllpassPlugin[] = { "http://example.org/categories/allpass", nullptr };
const char* const kCategoryBandpassPlugin[] = { "http://example.org/categories/bandpass", nullptr };
const char* const kCategoryCombPlugin[] = { "http://example.org/categories/comb", nullptr };
const char* const kCategoryEQPlugin[] = { "http://example.org/categories/eq", nullptr };
const char* const kCategoryMultiEQPlugin[] = { "http://example.org/categories/multieq", nullptr };
const char* const kCategoryParaEQPlugin[] = { "http://example.org/categories/paraeq", nullptr };
const char* const kCategoryHighpassPlugin[] = { "http://example.org/categories/highpass", nullptr };
const char* const kCategoryLowpassPlugin[] = { "http://example.org/categories/lowpass", nullptr };
const char* const kCategoryGeneratorPlugin[] = { "http://example.org/categories/generator", nullptr };
const char* const kCategoryConstantPlugin[] = { "http://example.org/categories/constant", nullptr };
const char* const kCategoryInstrumentPlugin[] = { "http://example.org/categories/instrument", nullptr };
const char* const kCategoryOscillatorPlugin[] = { "http://example.org/categories/oscillator", nullptr };
const char* const kCategoryModulatorPlugin[] = { "http://example.org/categories/modulator", nullptr };
const char* const kCategoryChorusPlugin[] = { "http://example.org/categories/chorus", nullptr };
const char* const kCategoryFlangerPlugin[] = { "http://example.org/categories/flanger", nullptr };
const char* const kCategoryPhaserPlugin[] = { "http://example.org/categories/phaser", nullptr };
const char* const kCategoryReverbPlugin[] = { "http://example.org/categories/reverb", nullptr };
const char* const kCategorySimulatorPlugin[] = { "http://example.org/categories/simulator", nullptr };
const char* const kCategorySpatialPlugin[] = { "http://example.org/categories/spatial", nullptr };
const char* const kCategorySpectralPlugin[] = { "http://example.org/categories/spectral", nullptr };
const char* const kCategoryPitchPlugin[] = { "http://example.org/categories/pitch", nullptr };
const char* const kCategoryUtilityPlugin[] = { "http://example.org/categories/utility", nullptr };
const char* const kCategoryAnalyserPlugin[] = { "http://example.org/categories/analyser", nullptr };
const char* const kCategoryConverterPlugin[] = { "http://example.org/categories/converter", nullptr };
const char* const kCategoryFunctionPlugin[] = { "http://example.org/categories/function", nullptr };
const char* const kCategoryMixerPlugin[] = { "http://example.org/categories/mixer", nullptr };
const char* const kCategoryMIDIPlugin[] = { "http://example.org/categories/midi", nullptr };
const char* const kCategoryMIDIPluginMOD[] = { "http://example.org/categories/midi-mod", nullptr };
const char* const kCategoryMaxGenPluginMOD[] = { "http://example.org/categories/maxgen-mod", nullptr };
const char* const kCategoryCamomilePluginMOD[] = { "http://example.org/categories/camomile-mod", nullptr };
const char* const kCategoryControlVoltagePluginMOD[] = { "http://example.org/categories/cv-mod", nullptr };

const char* const kBuildEnvironmentProd = "prod";
const char* const kBuildEnvironmentDev = "dev";
const char* const kBuildEnvironmentLabs = "labs";

const char* const kStabilityExperimental = "experimental";
const char* const kStabilityStable = "stable";
const char* const kStabilityTesting = "testing";

const char* const kUnit_s[] = { "s", nullptr };
const char* const kUnit_ms[] = { "ms", nullptr };
const char* const kUnit_min[] = { "min", nullptr };
const char* const kUnit_bar[] = { "bar", nullptr };
const char* const kUnit_beat[] = { "beat", nullptr };
const char* const kUnit_frame[] = { "frame", nullptr };
const char* const kUnit_m[] = { "m", nullptr };
const char* const kUnit_cm[] = { "cm", nullptr };
const char* const kUnit_mm[] = { "mm", nullptr };
const char* const kUnit_km[] = { "km", nullptr };
const char* const kUnit_inch[] = { "in", nullptr };
const char* const kUnit_mile[] = { "mile", nullptr };
const char* const kUnit_db[] = { "dB", nullptr };
const char* const kUnit_pc[] = { "%", nullptr };
const char* const kUnit_coef[] = { "coef", nullptr };
const char* const kUnit_hz[] = { "Hz", nullptr };
const char* const kUnit_khz[] = { "kHz", nullptr };
const char* const kUnit_mhz[] = { "MHz", nullptr };
const char* const kUnit_bpm[] = { "BPM", nullptr };
const char* const kUnit_oct[] = { "oct", nullptr };
const char* const kUnit_cent[] = { "cent", nullptr };
const char* const kUnit_semitone12TET[] = { "12tet", nullptr };
const char* const kUnit_degree[] = { "deg", nullptr };
const char* const kUnit_midiNote[] = { "MIDI", nullptr };
const char* const kUnit_volts[] = { "V", nullptr };

const char nc[1] = { '\0' };
