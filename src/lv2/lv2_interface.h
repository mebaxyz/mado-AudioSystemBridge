#pragma once

#include "lv2_constants.h"
#include "namespace_definitions.h"
#include <memory>

struct LilvWorld; // forward-declared, concrete type is from lilv

class LV2PluginScanner; // forward

class LV2Interface {
public:
    LV2Interface();
    ~LV2Interface();

    // Initialize the underlying lilv world and subsystems. Returns true on success.
    bool init();

    // Accessors
    NamespaceDefinitions& ns();

    LV2PluginScanner* pluginScanner();

    // Non-copyable
    LV2Interface(const LV2Interface&) = delete;
    LV2Interface& operator=(const LV2Interface&) = delete;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_; 
};
