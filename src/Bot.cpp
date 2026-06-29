#include "tgbotcpp/Bot.hpp"

#include <utility>

#include "tgbotcpp/net/OpenSslHttpClient.hpp"

namespace tgbotcpp {

struct Bot::Impl {
    net::OpenSslHttpClient http;
    Api api;

    explicit Impl(std::string token) : api(std::move(token), http) {}
};

Bot::Bot(std::string token) : impl_(std::make_unique<Impl>(std::move(token))) {}

Bot::~Bot() = default;

Api& Bot::api() {
    return impl_->api;
}

const Api& Bot::api() const {
    return impl_->api;
}

} // namespace tgbotcpp
