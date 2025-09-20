#pragma once

#include <string>
#include <vector>

// Common LV2-related constants and small types moved out of utils_lilv.cpp

enum PortDirection {
    kPortDirectionNull,
    kPortDirectionInput,
    kPortDirectionOutput,
};

enum PortType {
    kPortTypeNull,
    kPortTypeAudio,
    kPortTypeControl,
    kPortTypeCV,
    kPortTypeMIDI,
};

// Categories
extern const char* const kCategoryDelayPlugin[];
extern const char* const kCategoryDistortionPlugin[];
extern const char* const kCategoryWaveshaperPlugin[];
extern const char* const kCategoryDynamicsPlugin[];
extern const char* const kCategoryAmplifierPlugin[];
extern const char* const kCategoryCompressorPlugin[];
extern const char* const kCategoryExpanderPlugin[];
extern const char* const kCategoryGatePlugin[];
extern const char* const kCategoryLimiterPlugin[];
extern const char* const kCategoryFilterPlugin[];
extern const char* const kCategoryAllpassPlugin[];
extern const char* const kCategoryBandpassPlugin[];
extern const char* const kCategoryCombPlugin[];
extern const char* const kCategoryEQPlugin[];
extern const char* const kCategoryMultiEQPlugin[];
extern const char* const kCategoryParaEQPlugin[];
extern const char* const kCategoryHighpassPlugin[];
extern const char* const kCategoryLowpassPlugin[];
extern const char* const kCategoryGeneratorPlugin[];
extern const char* const kCategoryConstantPlugin[];
extern const char* const kCategoryInstrumentPlugin[];
extern const char* const kCategoryOscillatorPlugin[];
extern const char* const kCategoryModulatorPlugin[];
extern const char* const kCategoryChorusPlugin[];
extern const char* const kCategoryFlangerPlugin[];
extern const char* const kCategoryPhaserPlugin[];
extern const char* const kCategoryReverbPlugin[];
extern const char* const kCategorySimulatorPlugin[];
extern const char* const kCategorySpatialPlugin[];
extern const char* const kCategorySpectralPlugin[];
extern const char* const kCategoryPitchPlugin[];
extern const char* const kCategoryUtilityPlugin[];
extern const char* const kCategoryAnalyserPlugin[];
extern const char* const kCategoryConverterPlugin[];
extern const char* const kCategoryFunctionPlugin[];
extern const char* const kCategoryMixerPlugin[];
extern const char* const kCategoryMIDIPlugin[];
extern const char* const kCategoryMIDIPluginMOD[];
extern const char* const kCategoryMaxGenPluginMOD[];
extern const char* const kCategoryCamomilePluginMOD[];
extern const char* const kCategoryControlVoltagePluginMOD[];

// Build env / stability
extern const char* const kBuildEnvironmentProd;
extern const char* const kBuildEnvironmentDev;
extern const char* const kBuildEnvironmentLabs;

extern const char* const kStabilityExperimental;
extern const char* const kStabilityStable;
extern const char* const kStabilityTesting;

// Units (label, format, symbol)
extern const char* const kUnit_s[];
extern const char* const kUnit_ms[];
extern const char* const kUnit_min[];
extern const char* const kUnit_bar[];
extern const char* const kUnit_beat[];
extern const char* const kUnit_frame[];
extern const char* const kUnit_m[];
extern const char* const kUnit_cm[];
extern const char* const kUnit_mm[];
extern const char* const kUnit_km[];
extern const char* const kUnit_inch[];
extern const char* const kUnit_mile[];
extern const char* const kUnit_db[];
extern const char* const kUnit_pc[];
extern const char* const kUnit_coef[];
extern const char* const kUnit_hz[];
extern const char* const kUnit_khz[];
extern const char* const kUnit_mhz[];
extern const char* const kUnit_bpm[];
extern const char* const kUnit_oct[];
extern const char* const kUnit_cent[];
extern const char* const kUnit_semitone12TET[];
extern const char* const kUnit_degree[];
extern const char* const kUnit_midiNote[];
extern const char* const kUnit_volts[];

extern const char nc[1];
