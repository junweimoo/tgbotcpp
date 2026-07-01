#pragma once

#include <string>

#include "tgbotcpp/types/User.hpp"

namespace tgbotcpp {

/// Mirrors the Telegram `InlineQuery` object — an incoming inline-mode query.
struct InlineQuery {
    std::string id;
    User from;
    std::string query;
    std::string offset;
    std::string chatType;  // "sender" | "private" | "group" | "supergroup" | "channel"
    // `location` (Location) omitted until that type is modeled.
};

} // namespace tgbotcpp
