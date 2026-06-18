#pragma once

#include <string>

namespace tgbotcpp::net {

/// Abstract HTTP transport used by Api to talk to the Telegram servers.
/// Concrete implementations (curl, asio, ...) live under src/net.
class HttpClient {
public:
    virtual ~HttpClient() = default;

    virtual std::string get(const std::string& url) = 0;
    virtual std::string post(const std::string& url, const std::string& body) = 0;
};

} // namespace tgbotcpp::net
