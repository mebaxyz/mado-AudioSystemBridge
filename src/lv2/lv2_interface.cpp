#include "lv2_interface.h"
#include "namespace_definitions.h"
#include <memory>

// We keep the Lilv includes out of the public header to avoid forcing consumers
// to have lilv dev headers in their include path. The PIMPL will include them
// when building in an environment with lilv available.

struct LV2Interface::Impl {
    Impl() = default;
    ~Impl() = default;

    // These pointers are null if lilv is not present at build time in this environment.
    // When available these should be set during init().
    NamespaceDefinitions ns;
};

LV2Interface::LV2Interface() : impl_(new Impl()) {}

LV2Interface::~LV2Interface() = default;

bool LV2Interface::init() {
    // Defer actual world initialization to the plugin scanner or calling code.
    // Here we just create the namespace definitions container; the real init
    // will be done when a LilvWorld* is available.
    return true;
}

NamespaceDefinitions& LV2Interface::ns() { return impl_->ns; }

LV2PluginScanner* LV2Interface::pluginScanner() { return nullptr; }
