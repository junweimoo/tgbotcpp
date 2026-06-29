#pragma once

#include <string>

namespace tgbotcpp::net {

struct ParsedUrl {
    std::string host;
    std::string port;    // numeric service, e.g. "443"
    std::string target;  // path + query, always starts with '/'
};

/// Split an absolute `http://` or `https://` URL into host/port/target. Port
/// defaults to 443 (https) or 80 (http) when not present; an empty path becomes
/// "/". Throws std::runtime_error for non-http(s) schemes or malformed input.
ParsedUrl parseUrl(const std::string& url);

} // namespace tgbotcpp::net
