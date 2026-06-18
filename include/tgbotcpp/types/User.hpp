#pragma once

#include <cstdint>
#include <string>

namespace tgbotcpp {

/// Mirrors the Telegram `User` object.
struct User {
    std::int64_t id = 0;
    bool isBot = false;
    std::string firstName;
    std::string lastName;
    std::string username;
};

} // namespace tgbotcpp
