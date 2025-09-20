#include "lv2_plugin_scanner.h"

// We include the legacy C functions as extern "C" so we can delegate to them
extern "C" {
    #include "utils.h"

    // Legacy C API functions from utils_lilv.cpp
    bool is_bundle_loaded(const char* bundle);
    const char* const* add_bundle_to_lilv_world(const char* bundle);
    const char* const* remove_bundle_from_lilv_world(const char* bundle, const char* resource);
    const char* const* get_plugin_list(void);
    const PluginInfo_Mini* const* get_all_plugins(void);
    const PluginInfo* get_plugin_info(const char* uri);
    const PluginInfo_Essentials* get_plugin_info_essentials(const char* uri);
    void init(void);
    void _refresh(void);
    bool plugin_should_reload(const char* uri);
    void plugin_remove_from_reload(const char* uri);
    const LilvPlugins* get_lilv_plugins();
}

#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <list>

#include "namespace_definitions.h"

// Forward declare the internal legacy world getter if needed
extern "C" void init(void);
extern "C" void _refresh(void);

// Forward declare helper implemented in utils_lilv.cpp (migrated from static)
void _fill_bundles_for_plugin(std::list<std::string>& bundles, const LilvPlugin* const p, LilvNode* const pset_Preset);

LV2PluginScanner::LV2PluginScanner(LilvWorld* world) : world_(world) {}

LV2PluginScanner::~LV2PluginScanner() = default;

// Global singleton used during incremental migration - keeps previous global state accessible
LV2PluginScanner& LV2PluginScanner::global()
{
    static LV2PluginScanner inst(nullptr);
    return inst;
}

bool LV2PluginScanner::pluginShouldReload(const std::string& uri) const
{
    return std::find(pluginsToReload_.begin(), pluginsToReload_.end(), uri) != pluginsToReload_.end();
}

void LV2PluginScanner::pluginRemoveFromReload(const std::string& uri)
{
    pluginsToReload_.remove(uri);
}

void LV2PluginScanner::pluginAddToReload(const std::string& uri)
{
    if (std::find(pluginsToReload_.begin(), pluginsToReload_.end(), uri) == pluginsToReload_.end())
        pluginsToReload_.push_back(uri);
}

void LV2PluginScanner::setPlugins(const LilvPlugins* plugins)
{
    plugins_ = plugins;
}

const LilvPlugins* LV2PluginScanner::getPlugins() const
{
    return plugins_;
}

bool LV2PluginScanner::hasPluginInfo(const std::string& uri) const
{
    return plugInfo_.find(uri) != plugInfo_.end();
}

PluginInfo& LV2PluginScanner::getPluginInfoRef(const std::string& uri)
{
    return plugInfo_[uri];
}

void LV2PluginScanner::setPluginInfo(const std::string& uri, const PluginInfo& info)
{
    plugInfo_[uri] = info;
}

const PluginInfo_Mini* LV2PluginScanner::getPluginMini(const std::string& uri) const
{
    auto it = plugInfoMini_.find(uri);
    if (it == plugInfoMini_.end()) return nullptr;
    return it->second;
}

void LV2PluginScanner::setPluginMini(const std::string& uri, const PluginInfo_Mini* mini)
{
    plugInfoMini_[uri] = mini;
}

void LV2PluginScanner::clearPluginInfo(const std::string& uri)
{
    auto it = plugInfo_.find(uri);
    if (it != plugInfo_.end()) {
        _clear_plugin_info(it->second);
        plugInfo_.erase(it);
    }

    auto it2 = plugInfoMini_.find(uri);
    if (it2 != plugInfoMini_.end()) {
        if (it2->second) _clear_plugin_info_mini(it2->second);
        plugInfoMini_.erase(it2);
    }
}

void LV2PluginScanner::clearCaches()
{
    for (auto &kv : plugInfoMini_)
        if (kv.second) _clear_plugin_info_mini(kv.second);
    plugInfoMini_.clear();

    for (auto &kv : plugInfo_)
        _clear_plugin_info(kv.second);
    plugInfo_.clear();

    bundles_.clear();
    pluginsToReload_.clear();
}

PluginInfo* LV2PluginScanner::getPluginInfoPtr(const std::string& uri)
{
    auto it = plugInfo_.find(uri);
    if (it == plugInfo_.end()) return nullptr;
    return &it->second;
}

bool LV2PluginScanner::isBundleLoaded(const std::string& bundle) const {
    return is_bundle_loaded(bundle.c_str());
}

std::vector<std::string> LV2PluginScanner::addBundle(const std::string& bundle) {
    std::vector<std::string> added;
    const char* const* ret = add_bundle_to_lilv_world(bundle.c_str());
    if (!ret) return added;
    for (int i=0; ret[i] != nullptr; ++i)
        added.emplace_back(ret[i]);
    return added;
}

std::vector<std::string> LV2PluginScanner::removeBundle(const std::string& bundle, const std::string& resource) {
    std::vector<std::string> removed;
    const char* const* ret = remove_bundle_from_lilv_world(bundle.c_str(), resource.empty() ? nullptr : resource.c_str());
    if (!ret) return removed;
    for (int i=0; ret[i] != nullptr; ++i)
        removed.emplace_back(ret[i]);
    return removed;
}

std::vector<std::string> LV2PluginScanner::getPluginList() const {
    std::vector<std::string> list;
    const char* const* ret = get_plugin_list();
    if (!ret) return list;
    for (int i=0; ret[i] != nullptr; ++i)
        list.emplace_back(ret[i]);
    return list;
}

std::vector<const PluginInfo_Mini*> LV2PluginScanner::getAllPlugins() {
    std::vector<const PluginInfo_Mini*> list;
    const PluginInfo_Mini* const* ret = get_all_plugins();
    if (!ret) return list;
    for (int i=0; ret[i] != nullptr; ++i)
        list.push_back(ret[i]);
    return list;
}

const PluginInfo* LV2PluginScanner::getPluginInfo(const std::string& uri) const {
    return get_plugin_info(uri.c_str());
}

const PluginInfo_Essentials* LV2PluginScanner::getPluginInfoEssentials(const std::string& uri) const {
    return get_plugin_info_essentials(uri.c_str());
}

void LV2PluginScanner::refresh() {
    // Delegate to legacy init/_refresh behaviour
    _refresh();
}

void LV2PluginScanner::populateCaches(LilvWorld* world)
{
    // Populate internal caches from the Lilv world. This migrates the
    // per-plugin loop that used to live in utils_lilv::_refresh().
    if (world == nullptr)
        return;

    // Clear existing caches
    bundles_.clear();
    plugInfo_.clear();
    // free any allocated PluginInfo_Mini pointers previously stored
    for (auto &kv : plugInfoMini_) {
        delete kv.second;
    }
    plugInfoMini_.clear();

    const LilvPlugins* plugins = lilv_world_get_all_plugins(world);
    plugins_ = plugins;

    LilvNode* const pset_Preset = lilv_new_uri(world, LV2_PRESETS__Preset);

    LILV_FOREACH(plugins, itpls, plugins)
    {
        const LilvPlugin* const p = lilv_plugins_get(plugins, itpls);
        const std::string uri = lilv_node_as_uri(lilv_plugin_get_uri(p));

        // store empty dict for later
        plugInfo_[uri] = PluginInfo_Init;
        plugInfoMini_[uri] = nullptr;

        // fill bundles for this plugin
        _fill_bundles_for_plugin(bundles_, p, pset_Preset);
    }

    lilv_node_free(pset_Preset);
}

const PluginInfo_Mini* LV2PluginScanner::getPluginInfoMini(LilvWorld* const w, const LilvPlugin* const p, const NamespaceDefinitions& ns) const
{
    // Ported from utils_lilv.cpp::_get_plugin_info_mini
    const char* const uri = lilv_node_as_uri(lilv_plugin_get_uri(p));

    bool supported = true;

    for (unsigned int i=0, numports=lilv_plugin_get_num_ports(p); i<numports; ++i)
    {
        const LilvPort* const port = lilv_plugin_get_port_by_index(p, i);

        if (LilvNodes* const typenodes = lilv_port_get_value(p, port, ns.rdf_type))
        {
            bool hasDirection = false;
            bool isGood = false;

            LILV_FOREACH(nodes, it, typenodes)
            {
                const char* const typestr = lilv_node_as_string(lilv_nodes_get(typenodes, it));

                if (typestr == nullptr)
                    continue;

                if (strcmp(typestr, LV2_CORE__Port) == 0)
                    continue;

                if (strcmp(typestr, LV2_CORE__InputPort) == 0) {
                    hasDirection = true;
                    continue;
                }
                if (strcmp(typestr, LV2_CORE__OutputPort) == 0) {
                    hasDirection = true;
                    continue;
                }

                if (strcmp(typestr, LV2_MORPH__MorphPort) == 0)
                    continue;

                if (strcmp(typestr, LV2_CORE__AudioPort) == 0) { isGood = true; continue; }
                if (strcmp(typestr, LV2_CORE__ControlPort) == 0) { isGood = true; continue; }
                if (strcmp(typestr, LV2_CORE__CVPort) == 0) { isGood = true; continue; }
                if (strcmp(typestr, LV2_ATOM__AtomPort) == 0) { isGood = true; continue; }
                if (strcmp(typestr, MOD__CVPort) == 0) { isGood = true; continue; }
            }
            lilv_nodes_free(typenodes);

            if (! (hasDirection && isGood))
                supported = false;
        }
    }

    if (! supported)
    {
        printf("Plugin '%s' uses non-supported port types\n", uri);
        return nullptr;
    }

    const char* const* const category = getPluginCategories(p, ns.rdf_type, &supported);

    if (! supported)
        return nullptr;

    PluginInfo_Mini* const info = new PluginInfo_Mini;
    info->uri = uri;
    info->category = category;

    if (LilvNode* const node = lilv_plugin_get_name(p))
    {
        if (const char* const name = lilv_node_as_string(node))
            info->name = strdup(name);
        else
            info->name = nc;

        lilv_node_free(node);
    }
    else
    {
        info->name = nc;
    }

    char brand[16+1] = {};

    if (LilvNodes* const nodes = lilv_plugin_get_value(p, ns.mod_brand))
    {
        strncpy(brand, lilv_node_as_string(lilv_nodes_get_first(nodes)), 16);
        info->brand = strdup(brand);
        lilv_nodes_free(nodes);
    }
    else if (LilvNode* const node = lilv_plugin_get_author_name(p))
    {
        strncpy(brand, lilv_node_as_string(node), 16);
        info->brand = strdup(brand);
        lilv_node_free(node);
    }
    else
    {
        info->brand = nc;
    }

    char label[24+1] = {};

    if (LilvNodes* const nodes = lilv_plugin_get_value(p, ns.mod_label))
    {
        strncpy(label, lilv_node_as_string(lilv_nodes_get_first(nodes)), 24);
        info->label = strdup(label);
        lilv_nodes_free(nodes);
    }
    else if (info->name != nc)
    {
        if (strlen(info->name) <= 24)
            info->label = strdup(info->name);
        else
        {
            strncpy(label, info->name, 24);
            label[24] = '\0';
            info->label = strdup(label);
        }
    }
    else
    {
        info->label = nc;
    }

    // other fields are left as defaults in the migrated mini info
    info->valid = true;
    info->microVersion = 0;
    info->minorVersion = 0;
    info->release = 0;
    info->builder = 0;
    info->licensed = 0;
    info->iotype = kPluginIONull;

    return info;
}

const char* const* LV2PluginScanner::getPluginCategories(const LilvPlugin* p, LilvNode* rdf_type, bool* supported) const
{
    const char* const* category = nullptr;

    if (LilvNodes* const nodes = lilv_plugin_get_value(p, rdf_type))
    {
        LILV_FOREACH(nodes, it, nodes)
        {
            const LilvNode* const node2 = lilv_nodes_get(nodes, it);
            const char* const nodestr = lilv_node_as_string(node2);

            if (nodestr == nullptr)
                continue;

            if (strcmp(nodestr, LILV_NS_MODPEDAL "Pedalboard") == 0)
            {
                if (supported != nullptr)
                    *supported = false;
                category = nullptr;
                break;
            }

            if (const char* cat = strstr(nodestr, "http://lv2plug.in/ns/lv2core#"))
            {
                cat += 29; // strlen("http://lv2plug.in/ns/lv2core#")

                if (cat[0] == '\0')
                    continue;
                if (strcmp(cat, "Plugin") == 0)
                    continue;

                else if (strcmp(cat, "DelayPlugin") == 0)
                    category = kCategoryDelayPlugin;
                else if (strcmp(cat, "DistortionPlugin") == 0)
                    category = kCategoryDistortionPlugin;
                else if (strcmp(cat, "WaveshaperPlugin") == 0)
                    category = kCategoryWaveshaperPlugin;
                else if (strcmp(cat, "DynamicsPlugin") == 0)
                    category = kCategoryDynamicsPlugin;
                else if (strcmp(cat, "AmplifierPlugin") == 0)
                    category = kCategoryAmplifierPlugin;
                else if (strcmp(cat, "CompressorPlugin") == 0)
                    category = kCategoryCompressorPlugin;
                else if (strcmp(cat, "ExpanderPlugin") == 0)
                    category = kCategoryExpanderPlugin;
                else if (strcmp(cat, "GatePlugin") == 0)
                    category = kCategoryGatePlugin;
                else if (strcmp(cat, "LimiterPlugin") == 0)
                    category = kCategoryLimiterPlugin;
                else if (strcmp(cat, "FilterPlugin") == 0)
                    category = kCategoryFilterPlugin;
                else if (strcmp(cat, "AllpassPlugin") == 0)
                    category = kCategoryAllpassPlugin;
                else if (strcmp(cat, "BandpassPlugin") == 0)
                    category = kCategoryBandpassPlugin;
                else if (strcmp(cat, "CombPlugin") == 0)
                    category = kCategoryCombPlugin;
                else if (strcmp(cat, "EQPlugin") == 0)
                    category = kCategoryEQPlugin;
                else if (strcmp(cat, "MultiEQPlugin") == 0)
                    category = kCategoryMultiEQPlugin;
                else if (strcmp(cat, "ParaEQPlugin") == 0)
                    category = kCategoryParaEQPlugin;
                else if (strcmp(cat, "HighpassPlugin") == 0)
                    category = kCategoryHighpassPlugin;
                else if (strcmp(cat, "LowpassPlugin") == 0)
                    category = kCategoryLowpassPlugin;
                else if (strcmp(cat, "GeneratorPlugin") == 0)
                    category = kCategoryGeneratorPlugin;
                else if (strcmp(cat, "ConstantPlugin") == 0)
                    category = kCategoryConstantPlugin;
                else if (strcmp(cat, "InstrumentPlugin") == 0)
                    category = kCategoryInstrumentPlugin;
                else if (strcmp(cat, "OscillatorPlugin") == 0)
                    category = kCategoryOscillatorPlugin;
                else if (strcmp(cat, "ModulatorPlugin") == 0)
                    category = kCategoryModulatorPlugin;
                else if (strcmp(cat, "ChorusPlugin") == 0)
                    category = kCategoryChorusPlugin;
                else if (strcmp(cat, "FlangerPlugin") == 0)
                    category = kCategoryFlangerPlugin;
                else if (strcmp(cat, "PhaserPlugin") == 0)
                    category = kCategoryPhaserPlugin;
                else if (strcmp(cat, "ReverbPlugin") == 0)
                    category = kCategoryReverbPlugin;
                else if (strcmp(cat, "SimulatorPlugin") == 0)
                    category = kCategorySimulatorPlugin;
                else if (strcmp(cat, "SpatialPlugin") == 0)
                    category = kCategorySpatialPlugin;
                else if (strcmp(cat, "SpectralPlugin") == 0)
                    category = kCategorySpectralPlugin;
                else if (strcmp(cat, "PitchPlugin") == 0)
                    category = kCategoryPitchPlugin;
                else if (strcmp(cat, "UtilityPlugin") == 0)
                    category = kCategoryUtilityPlugin;
                else if (strcmp(cat, "AnalyserPlugin") == 0)
                    category = kCategoryAnalyserPlugin;
                else if (strcmp(cat, "ConverterPlugin") == 0)
                    category = kCategoryConverterPlugin;
                else if (strcmp(cat, "FunctionPlugin") == 0)
                    category = kCategoryFunctionPlugin;
                else if (strcmp(cat, "MixerPlugin") == 0)
                    category = kCategoryMixerPlugin;
                else if (strcmp(cat, "MIDIPlugin") == 0)
                    category = kCategoryMIDIPlugin;
            }
            else if (const char* cat2 = strstr(nodestr, LILV_NS_MOD))
            {
                cat2 += 29; // strlen("http://moddevices.com/ns/mod#")

                if (cat2[0] == '\0')
                    continue;

                else if (strcmp(cat2, "DelayPlugin") == 0)
                    category = kCategoryDelayPlugin;
                else if (strcmp(cat2, "DistortionPlugin") == 0)
                    category = kCategoryDistortionPlugin;
                else if (strcmp(cat2, "DynamicsPlugin") == 0)
                    category = kCategoryDynamicsPlugin;
                else if (strcmp(cat2, "FilterPlugin") == 0)
                    category = kCategoryFilterPlugin;
                else if (strcmp(cat2, "GeneratorPlugin") == 0)
                    category = kCategoryGeneratorPlugin;
                else if (strcmp(cat2, "ModulatorPlugin") == 0)
                    category = kCategoryModulatorPlugin;
                else if (strcmp(cat2, "ReverbPlugin") == 0)
                    category = kCategoryReverbPlugin;
                else if (strcmp(cat2, "SimulatorPlugin") == 0)
                    category = kCategorySimulatorPlugin;
                else if (strcmp(cat2, "SpatialPlugin") == 0)
                    category = kCategorySpatialPlugin;
                else if (strcmp(cat2, "SpectralPlugin") == 0)
                    category = kCategorySpectralPlugin;
                else if (strcmp(cat2, "UtilityPlugin") == 0)
                    category = kCategoryUtilityPlugin;
                else if (strcmp(cat2, "MIDIPlugin") == 0)
                    category = kCategoryMIDIPluginMOD;
                else if (strcmp(cat2, "MaxGenPlugin") == 0)
                    category = kCategoryMaxGenPluginMOD;
                else if (strcmp(cat2, "CamomilePlugin") == 0)
                    category = kCategoryCamomilePluginMOD;
                else if (strcmp(cat2, "ControlVoltagePlugin") == 0)
                    category = kCategoryControlVoltagePluginMOD;
                else
                    continue; // invalid mod category

                // if we reach this point we found a mod category.
                // we need to stop now, as only 1 mod category is allowed per plugin.
                break;
            }
        }
        lilv_nodes_free(nodes);
    }

    return category;
}

const PluginInfo* LV2PluginScanner::fillPluginInfoWithPresets(LilvWorld* const w, PluginInfo& info, const std::string& uri)
{
    // Ported from utils_lilv.cpp::_fill_plugin_info_with_presets

    if (! plugin_should_reload(uri.c_str()))
        return &info;

    plugin_remove_from_reload(uri.c_str());

    LilvNode* const node = lilv_new_uri(w, uri.c_str());

    if (node == nullptr)
        return &info;

    const LilvPlugin* const p = lilv_plugins_get_by_uri(get_lilv_plugins(), node);
    lilv_node_free(node);

    if (p == nullptr)
        return &info;

    if (info.presets != nullptr)
    {
        for (int i=0; info.presets[i].valid; ++i)
        {
            free((void*)info.presets[i].uri);
            free((void*)info.presets[i].label);
            if (info.presets[i].path != nc)
                free((void*)info.presets[i].path);
        }
        delete[] info.presets;
        info.presets = nullptr;
    }

    LilvNode* const pset_Preset = lilv_new_uri(w, LV2_PRESETS__Preset);
    LilvNode* const rdfs_label  = lilv_new_uri(w, LILV_NS_RDFS "label");

    placePresetInfo(w, info, p, pset_Preset, rdfs_label);

    lilv_node_free(pset_Preset);
    lilv_node_free(rdfs_label);

    return &info;
}

void LV2PluginScanner::placePresetInfo(LilvWorld* const w, PluginInfo& info, const LilvPlugin* p, LilvNode* const pset_Preset, LilvNode* const rdfs_label) const
{
    // Ported from utils_lilv.cpp::_place_preset_info
    LilvNodes* const presetnodes = lilv_plugin_get_related(p, pset_Preset);

    if (presetnodes == nullptr)
        return;

    const unsigned int presetcount = lilv_nodes_size(presetnodes);
    unsigned int prindex = 0;

    PluginPreset* const presets = new PluginPreset[presetcount+1];
    memset(presets, 0, sizeof(PluginPreset) * (presetcount+1));

    char lastSeenBundle[0xff];
    lastSeenBundle[0] = lastSeenBundle[0xff-1] = '\0';

    const char* const mainBundle(info.bundles[0]);

    std::vector<const LilvNode*> loadedPresetResourceNodes;

    LILV_FOREACH(nodes, itprs, presetnodes)
    {
        if (prindex >= presetcount)
            continue;

        const LilvNode* const presetnode = lilv_nodes_get(presetnodes, itprs);

        LilvNode* xlabel = lilv_world_get(w, presetnode, rdfs_label, nullptr);

        if (xlabel == nullptr)
        {
            if (lilv_world_load_resource(w, presetnode) == -1)
                continue;

            xlabel = lilv_world_get(w, presetnode, rdfs_label, nullptr);
            loadedPresetResourceNodes.push_back(presetnode);
        }

        if (xlabel != nullptr)
        {
            const char* const preseturi  = lilv_node_as_uri(presetnode);
            const char*       presetpath = nc;

            if (strncmp(preseturi, "file://", 7) == 0)
            {
                if (char* const lilvparsed = lilv_file_uri_parse2(preseturi, nullptr))
                {
                    if (const char* bundlepath = dirname(lilvparsed))
                    {
                        if (lastSeenBundle[0] != '\0' && strcmp(bundlepath, lastSeenBundle) == 0)
                        {
                            if (presets[prindex-1].path != nc)
                            {
                                free((void*)presets[prindex-1].path);
                                presets[prindex-1].path = nc;
                            }
                        }
                        else
                        {
                            strncpy(lastSeenBundle, bundlepath, 0xff-1);

                            size_t bundlepathsize;
                            bundlepath = _get_safe_bundlepath(bundlepath, bundlepathsize);

                            if (strcmp(mainBundle, bundlepath) != 0)
                                presetpath = strdup(bundlepath);
                        }
                    }

                    lilv_free(lilvparsed);
                }
            }

            presets[prindex++] = {
                true,
                strdup(preseturi),
                strdup(lilv_node_as_string(xlabel)),
                presetpath
            };

            lilv_node_free(xlabel);
        }
    }

    if (prindex > 1)
        _sort_presets_data(presets, prindex);

#ifdef HAVE_NEW_LILV
    for (const LilvNode* presetnode : loadedPresetResourceNodes)
        lilv_world_unload_resource(w, presetnode);
#endif

    info.presets = presets;

    loadedPresetResourceNodes.clear();
    lilv_nodes_free(presetnodes);
}
