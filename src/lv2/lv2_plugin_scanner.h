#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include "lv2_constants.h"

// Forward declarations from Lilv and project utils
struct LilvWorld;
struct LilvPlugin;
#include "namespace_definitions.h"
struct PluginInfo;
struct PluginInfo_Mini;
struct PluginInfo_Essentials;

class LV2PluginScanner {
public:
    explicit LV2PluginScanner(LilvWorld* world = nullptr);
    ~LV2PluginScanner();

    // Non-copyable
    LV2PluginScanner(const LV2PluginScanner&) = delete;
    LV2PluginScanner& operator=(const LV2PluginScanner&) = delete;

    // Bundle management
    bool isBundleLoaded(const std::string& bundle) const;
    std::vector<std::string> addBundle(const std::string& bundle);
    std::vector<std::string> removeBundle(const std::string& bundle, const std::string& resource = "");

    // Plugin enumeration / lookup
    std::vector<std::string> getPluginList() const;
    std::vector<const PluginInfo_Mini*> getAllPlugins();
    const PluginInfo* getPluginInfo(const std::string& uri) const;
    const PluginInfo_Essentials* getPluginInfoEssentials(const std::string& uri) const;

    // Refresh the internal Lilv world snapshot (delegates to global init/_refresh in legacy code)
    void refresh();

    // Populate internal caches (migrated from legacy _refresh() loop)
    void populateCaches(LilvWorld* world);

    // Global singleton for incremental migration (legacy C code can use wrappers to forward here)
    static LV2PluginScanner& global();

    // Cache accessors (migrating globals into the class)
    bool pluginShouldReload(const std::string& uri) const;
    void pluginRemoveFromReload(const std::string& uri);
    void pluginAddToReload(const std::string& uri);

    void setPlugins(const LilvPlugins* plugins);
    const LilvPlugins* getPlugins() const;

    // Accessors for legacy-style plugin caches (migration helpers)
    bool hasPluginInfo(const std::string& uri) const;
    PluginInfo& getPluginInfoRef(const std::string& uri);
    void setPluginInfo(const std::string& uri, const PluginInfo& info);

    const PluginInfo_Mini* getPluginMini(const std::string& uri) const;
    void setPluginMini(const std::string& uri, const PluginInfo_Mini* mini);

    // Removal/cleanup helpers
    void clearPluginInfo(const std::string& uri);
    void clearCaches();

    // Pointer accessors (return nullptr if not present)
    PluginInfo* getPluginInfoPtr(const std::string& uri);

    // Incremental helpers (migrated from utils_lilv.cpp)
    const PluginInfo_Mini* getPluginInfoMini(LilvWorld* const w, const LilvPlugin* const p, const NamespaceDefinitions& ns) const;
    const PluginInfo* fillPluginInfoWithPresets(LilvWorld* const w, PluginInfo& info, const std::string& uri);
    void placePresetInfo(LilvWorld* const w, PluginInfo& info, const LilvPlugin* p, LilvNode* const pset_Preset, LilvNode* const rdfs_label) const;
    const char* const* getPluginCategories(const LilvPlugin* p, LilvNode* rdf_type, bool* supported = nullptr) const;

private:
    LilvWorld* world_;
    // migrated caches
    const LilvPlugins* plugins_ = nullptr;
    std::unordered_map<std::string, PluginInfo> plugInfo_;
    std::unordered_map<std::string, const PluginInfo_Mini*> plugInfoMini_;
    std::list<std::string> pluginsToReload_;
    std::list<std::string> bundles_;
};
