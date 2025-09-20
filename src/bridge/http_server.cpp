#include <iostream>
#include <string>
#include <thread>
#include "../lv2/lv2_plugin_scanner.h"
#include "jack/jack_manager.h"

// Single-header HTTP library (https://github.com/yhirose/cpp-httplib)
#include "httplib.h"

using namespace std;

int main(int argc, char** argv) {
    int port = 8080;
    if (argc > 1) port = stoi(argv[1]);

    httplib::Server svr;

    svr.Get("/health", [](const httplib::Request&, httplib::Response& res){
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    svr.Get(R"(/plugins)", [&](const httplib::Request&, httplib::Response& res){
        vector<const PluginInfo_Mini*> all = LV2PluginScanner::global().getAllPlugins();
        string out = "[";
        bool first = true;
        for (const PluginInfo_Mini* p : all) {
            if (!p) continue;
            if (!first) out += ",";
            first = false;
            out += "{\"uri\":\"" + string(p->uri) + "\",\"name\":\"" + string(p->name) + "\"}";
        }
        out += "]";
        res.set_content(out, "application/json");
    });

    svr.Get(R"(/plugins/(.*))", [&](const httplib::Request& req, httplib::Response& res){
        auto uri = req.matches[1];
        const PluginInfo* info = LV2PluginScanner::global().getPluginInfo(uri);
        if (!info) {
            res.status = 404;
            res.set_content("{\"error\":\"not found\"}", "application/json");
            return;
        }
        // Minimal JSON for demo purposes
        string out = "{\"uri\":\"" + string(info->uri) + "\",\"name\":\"" + string(info->name) + "\"}";
        res.set_content(out, "application/json");
    });

    cout << "Starting AudioSystemBridge HTTP server on port " << port << "\n";
    svr.listen("0.0.0.0", port);
    return 0;
}
