#pragma once

#include <lilv/lilv.h>

#ifndef MOD_LICENSE__interface
#define MOD_LICENSE__interface "http://moddevices.com/ns/ext/license#interface"
#endif

class NamespaceDefinitions {
public:
    NamespaceDefinitions() = default;
    explicit NamespaceDefinitions(LilvWorld* w);
    ~NamespaceDefinitions();

    void init(LilvWorld* w);
    void cleanup();

    // Public members kept for compatibility with existing code
    LilvNode* doap_license = nullptr;
    LilvNode* doap_maintainer = nullptr;
    LilvNode* foaf_homepage = nullptr;
    LilvNode* rdf_type = nullptr;
    LilvNode* rdfs_comment = nullptr;
    LilvNode* rdfs_label = nullptr;
    LilvNode* rdfs_range = nullptr;
    LilvNode* lv2core_designation = nullptr;
    LilvNode* lv2core_index = nullptr;
    LilvNode* lv2core_microVersion = nullptr;
    LilvNode* lv2core_minorVersion = nullptr;
    LilvNode* lv2core_name = nullptr;
    LilvNode* lv2core_project = nullptr;
    LilvNode* lv2core_portProperty = nullptr;
    LilvNode* lv2core_shortName = nullptr;
    LilvNode* lv2core_symbol = nullptr;
    LilvNode* lv2core_default = nullptr;
    LilvNode* lv2core_minimum = nullptr;
    LilvNode* lv2core_maximum = nullptr;
    LilvNode* lv2core_extensionData = nullptr;
    LilvNode* mod_brand = nullptr;
    LilvNode* mod_label = nullptr;
    LilvNode* mod_default = nullptr;
    LilvNode* mod_default_custom = nullptr;
    LilvNode* mod_minimum = nullptr;
    LilvNode* mod_maximum = nullptr;
    LilvNode* mod_rangeSteps = nullptr;
    LilvNode* mod_release = nullptr;
    LilvNode* mod_builder = nullptr;
    LilvNode* mod_buildEnvironment = nullptr;
    LilvNode* mod_fileTypes = nullptr;
    LilvNode* mod_supportedExtensions = nullptr;
    LilvNode* mod_rawMIDIClockAccess = nullptr;
    LilvNode* modlicense_interface = nullptr;
    LilvNode* modgui_gui = nullptr;
    LilvNode* modgui_resourcesDirectory = nullptr;
    LilvNode* modgui_iconTemplate = nullptr;
    LilvNode* modgui_settingsTemplate = nullptr;
    LilvNode* modgui_javascript = nullptr;
    LilvNode* modgui_stylesheet = nullptr;
    LilvNode* modgui_screenshot = nullptr;
    LilvNode* modgui_thumbnail = nullptr;
    LilvNode* modgui_discussionURL = nullptr;
    LilvNode* modgui_documentation = nullptr;
    LilvNode* modgui_brand = nullptr;
    LilvNode* modgui_label = nullptr;
    LilvNode* modgui_model = nullptr;
    LilvNode* modgui_panel = nullptr;
    LilvNode* modgui_color = nullptr;
    LilvNode* modgui_knob = nullptr;
    LilvNode* modgui_port = nullptr;
    LilvNode* modgui_monitoredOutputs = nullptr;
    LilvNode* atom_bufferType = nullptr;
    LilvNode* atom_Sequence = nullptr;
    LilvNode* midi_MidiEvent = nullptr;
    LilvNode* pprops_rangeSteps = nullptr;
    LilvNode* patch_readable = nullptr;
    LilvNode* patch_writable = nullptr;
    LilvNode* pset_Preset = nullptr;
    LilvNode* state_state = nullptr;
    LilvNode* units_render = nullptr;
    LilvNode* units_symbol = nullptr;
    LilvNode* units_unit = nullptr;

    bool initialized = false;

    // Prevent copy/move
    NamespaceDefinitions(const NamespaceDefinitions&) = delete;
    NamespaceDefinitions& operator=(const NamespaceDefinitions&) = delete;
};
