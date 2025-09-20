#include "namespace_definitions.h"

#include <cstring>

#ifndef LILV_NS_INGEN
#define LILV_NS_INGEN    "http://drobilla.net/ns/ingen#"
#endif
#ifndef LILV_NS_MOD
#define LILV_NS_MOD      "http://moddevices.com/ns/mod#"
#endif
#ifndef LILV_NS_MODGUI
#define LILV_NS_MODGUI   "http://moddevices.com/ns/modgui#"
#endif

NamespaceDefinitions::NamespaceDefinitions(LilvWorld* w)
{
    if (w)
        init(w);
}

NamespaceDefinitions::~NamespaceDefinitions()
{
    cleanup();
}

void NamespaceDefinitions::init(LilvWorld* const w)
{
    if (initialized || w == nullptr)
        return;
    initialized = true;

    doap_license             = lilv_new_uri(w, LILV_NS_DOAP "license");
    doap_maintainer          = lilv_new_uri(w, LILV_NS_DOAP "maintainer");
    foaf_homepage            = lilv_new_uri(w, LILV_NS_FOAF "homepage");
    rdf_type                 = lilv_new_uri(w, LILV_NS_RDF "type");
    rdfs_comment             = lilv_new_uri(w, LILV_NS_RDFS "comment");
    rdfs_label               = lilv_new_uri(w, LILV_NS_RDFS "label");
    rdfs_range               = lilv_new_uri(w, LILV_NS_RDFS "range");
    lv2core_designation      = lilv_new_uri(w, LILV_NS_LV2 "designation");
    lv2core_index            = lilv_new_uri(w, LILV_NS_LV2 "index");
    lv2core_microVersion     = lilv_new_uri(w, LILV_NS_LV2 "microVersion");
    lv2core_minorVersion     = lilv_new_uri(w, LILV_NS_LV2 "minorVersion");
    lv2core_name             = lilv_new_uri(w, LILV_NS_LV2 "name");
    lv2core_project          = lilv_new_uri(w, LILV_NS_LV2 "project");
    lv2core_portProperty     = lilv_new_uri(w, LILV_NS_LV2 "portProperty");
    lv2core_shortName        = lilv_new_uri(w, LILV_NS_LV2 "shortName");
    lv2core_symbol           = lilv_new_uri(w, LILV_NS_LV2 "symbol");
    lv2core_default          = lilv_new_uri(w, LILV_NS_LV2 "default");
    lv2core_minimum          = lilv_new_uri(w, LILV_NS_LV2 "minimum");
    lv2core_maximum          = lilv_new_uri(w, LILV_NS_LV2 "maximum");
    lv2core_extensionData    = lilv_new_uri(w, LILV_NS_LV2 "extensionData");
    mod_brand                = lilv_new_uri(w, LILV_NS_MOD "brand");
    mod_label                = lilv_new_uri(w, LILV_NS_MOD "label");
    mod_default              = lilv_new_uri(w, LILV_NS_MOD "default");
#if defined(_MOD_DEVICE_DUO)
    mod_default_custom       = lilv_new_uri(w, LILV_NS_MOD "default_duo");
#elif defined(_MOD_DEVICE_DUOX)
    mod_default_custom       = lilv_new_uri(w, LILV_NS_MOD "default_duox");
#elif defined(_MOD_DEVICE_DWARF)
    mod_default_custom       = lilv_new_uri(w, LILV_NS_MOD "default_dwarf");
#elif defined(_MOD_DEVICE_X86_64)
    mod_default_custom       = lilv_new_uri(w, LILV_NS_MOD "default_x64");
#else
    mod_default_custom       = nullptr;
#endif
    mod_minimum              = lilv_new_uri(w, LILV_NS_MOD "minimum");
    mod_maximum              = lilv_new_uri(w, LILV_NS_MOD "maximum");
    mod_rangeSteps           = lilv_new_uri(w, LILV_NS_MOD "rangeSteps");
    mod_release              = lilv_new_uri(w, LILV_NS_MOD "releaseNumber");
    mod_builder              = lilv_new_uri(w, LILV_NS_MOD "builderVersion");
    mod_buildEnvironment     = lilv_new_uri(w, LILV_NS_MOD "buildEnvironment");
    mod_fileTypes            = lilv_new_uri(w, LILV_NS_MOD "fileTypes");
    mod_supportedExtensions  = lilv_new_uri(w, LILV_NS_MOD "supportedExtensions");
    mod_rawMIDIClockAccess   = lilv_new_uri(w, LILV_NS_MOD "rawMIDIClockAccess");
    modlicense_interface     = lilv_new_uri(w, MOD_LICENSE__interface);
    modgui_gui               = lilv_new_uri(w, LILV_NS_MODGUI "gui");
    modgui_resourcesDirectory= lilv_new_uri(w, LILV_NS_MODGUI "resourcesDirectory");
    modgui_iconTemplate      = lilv_new_uri(w, LILV_NS_MODGUI "iconTemplate");
    modgui_settingsTemplate  = lilv_new_uri(w, LILV_NS_MODGUI "settingsTemplate");
    modgui_javascript        = lilv_new_uri(w, LILV_NS_MODGUI "javascript");
    modgui_stylesheet        = lilv_new_uri(w, LILV_NS_MODGUI "stylesheet");
    modgui_screenshot        = lilv_new_uri(w, LILV_NS_MODGUI "screenshot");
    modgui_thumbnail         = lilv_new_uri(w, LILV_NS_MODGUI "thumbnail");
    modgui_discussionURL     = lilv_new_uri(w, LILV_NS_MODGUI "discussionURL");
    modgui_documentation     = lilv_new_uri(w, LILV_NS_MODGUI "documentation");
    modgui_brand             = lilv_new_uri(w, LILV_NS_MODGUI "brand");
    modgui_label             = lilv_new_uri(w, LILV_NS_MODGUI "label");
    modgui_model             = lilv_new_uri(w, LILV_NS_MODGUI "model");
    modgui_panel             = lilv_new_uri(w, LILV_NS_MODGUI "panel");
    modgui_color             = lilv_new_uri(w, LILV_NS_MODGUI "color");
    modgui_knob              = lilv_new_uri(w, LILV_NS_MODGUI "knob");
    modgui_port              = lilv_new_uri(w, LILV_NS_MODGUI "port");
    modgui_monitoredOutputs  = lilv_new_uri(w, LILV_NS_MODGUI "monitoredOutputs");
    atom_bufferType          = lilv_new_uri(w, LV2_ATOM__bufferType);
    atom_Sequence            = lilv_new_uri(w, LV2_ATOM__Sequence);
    midi_MidiEvent           = lilv_new_uri(w, LV2_MIDI__MidiEvent);
    pprops_rangeSteps        = lilv_new_uri(w, LV2_PORT_PROPS__rangeSteps);
    patch_readable           = lilv_new_uri(w, LV2_PATCH__readable);
    patch_writable           = lilv_new_uri(w, LV2_PATCH__writable);
    pset_Preset              = lilv_new_uri(w, LV2_PRESETS__Preset);
    state_state              = lilv_new_uri(w, LV2_STATE__state);
    units_render             = lilv_new_uri(w, LV2_UNITS__render);
    units_symbol             = lilv_new_uri(w, LV2_UNITS__symbol);
    units_unit               = lilv_new_uri(w, LV2_UNITS__unit);
}

void NamespaceDefinitions::cleanup()
{
    if (!initialized)
        return;
    initialized = false;

    lilv_node_free(doap_license);
    lilv_node_free(doap_maintainer);
    lilv_node_free(foaf_homepage);
    lilv_node_free(rdf_type);
    lilv_node_free(rdfs_comment);
    lilv_node_free(rdfs_label);
    lilv_node_free(rdfs_range);
    lilv_node_free(lv2core_designation);
    lilv_node_free(lv2core_index);
    lilv_node_free(lv2core_microVersion);
    lilv_node_free(lv2core_minorVersion);
    lilv_node_free(lv2core_name);
    lilv_node_free(lv2core_project);
    lilv_node_free(lv2core_portProperty);
    lilv_node_free(lv2core_shortName);
    lilv_node_free(lv2core_symbol);
    lilv_node_free(lv2core_default);
    lilv_node_free(lv2core_minimum);
    lilv_node_free(lv2core_maximum);
    lilv_node_free(lv2core_extensionData);
    lilv_node_free(mod_brand);
    lilv_node_free(mod_label);
    lilv_node_free(mod_default);
    lilv_node_free(mod_default_custom);
    lilv_node_free(mod_minimum);
    lilv_node_free(mod_maximum);
    lilv_node_free(mod_rangeSteps);
    lilv_node_free(mod_release);
    lilv_node_free(mod_builder);
    lilv_node_free(mod_buildEnvironment);
    lilv_node_free(mod_fileTypes);
    lilv_node_free(mod_supportedExtensions);
    lilv_node_free(mod_rawMIDIClockAccess);
    lilv_node_free(modlicense_interface);
    lilv_node_free(modgui_gui);
    lilv_node_free(modgui_resourcesDirectory);
    lilv_node_free(modgui_iconTemplate);
    lilv_node_free(modgui_settingsTemplate);
    lilv_node_free(modgui_javascript);
    lilv_node_free(modgui_stylesheet);
    lilv_node_free(modgui_screenshot);
    lilv_node_free(modgui_thumbnail);
    lilv_node_free(modgui_discussionURL);
    lilv_node_free(modgui_documentation);
    lilv_node_free(modgui_brand);
    lilv_node_free(modgui_label);
    lilv_node_free(modgui_model);
    lilv_node_free(modgui_panel);
    lilv_node_free(modgui_color);
    lilv_node_free(modgui_knob);
    lilv_node_free(modgui_port);
    lilv_node_free(modgui_monitoredOutputs);
    lilv_node_free(atom_bufferType);
    lilv_node_free(atom_Sequence);
    lilv_node_free(midi_MidiEvent);
    lilv_node_free(pprops_rangeSteps);
    lilv_node_free(patch_readable);
    lilv_node_free(patch_writable);
    lilv_node_free(pset_Preset);
    lilv_node_free(state_state);
    lilv_node_free(units_render);
    lilv_node_free(units_symbol);
    lilv_node_free(units_unit);
}
