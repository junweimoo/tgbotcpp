#include "tgbotcpp/Bot.hpp"

#include "tgbotcpp/net/HttpClient.hpp"

namespace tgbotcpp {
namespace {

class NullHttpClient : public net::HttpClient {
public:
    std::string get(const std::string& /*url*/) override { return {}; }
    std::string post(const std::string& /*url*/, const std::string& /*body*/) override {
        return {};
    }
};

} // namespace

struct Bot::Impl {
    NullHttpClient http;
    Api api;

    explicit Impl(std::string token) : api(std::move(token), http) {}
};

Bot::Bot(std::string /*token*/) {
    // Implementation pending.
}

Bot::~Bot() = default;

} // namespace tgbotcpp
