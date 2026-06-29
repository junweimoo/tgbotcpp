#pragma once

#include <memory>
#include <string>

#include "tgbotcpp/net/HttpClient.hpp"

namespace tgbotcpp::net {

class OpenSslHttpClient : public HttpClient {
public:
    OpenSslHttpClient();
    ~OpenSslHttpClient() override;

    std::string get(const std::string& url) override;
    std::string post(const std::string& url, const std::string& body) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace tgbotcpp::net
