// Minimal placeholder for cpp-httplib (shim). Replace with upstream single-header for real use.
#pragma once
#include <string>
#include <functional>
#include <vector>

namespace httplib {
    struct Request {
        std::vector<std::string> matches;
    };
    struct Response {
        int status = 200;
        std::string body;
        void set_content(const std::string& s, const char*) { body = s; }
    };

    class Server {
    public:
        using Handler = std::function<void(const Request&, Response&)>;
        void Get(const std::string&, Handler) {}
        void listen(const char*, int) { /* no-op shim */ }
    };
}
