#include "tgbotcpp/Bot.hpp"

namespace tgbotcpp {

struct Bot::Impl {
    Api api;

    explicit Impl(std::string token) : api(std::move(token)) {}
};

Bot::Bot(std::string /*token*/) {
    // Implementation pending.
}

Bot::~Bot() = default;

} // namespace tgbotcpp
