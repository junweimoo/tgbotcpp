#pragma once

#include <cstdint>
#include <string>

namespace tgbotcpp {

/// Mirrors the Telegram `Chat` object.
struct Chat {
    std::int64_t id = 0;
    std::string type;   // "private", "group", "supergroup" or "channel"
    std::string title;
    std::string username;
};

} // namespace tgbotcpp
