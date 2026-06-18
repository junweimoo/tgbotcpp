#pragma once

#include <cstdint>
#include <string>

#include "tgbotcpp/types/Chat.hpp"
#include "tgbotcpp/types/User.hpp"

namespace tgbotcpp {

/// Mirrors the Telegram `Message` object.
struct Message {
    std::int64_t messageId = 0;
    std::int64_t date = 0;
    Chat chat;
    User from;
    std::string text;
};

} // namespace tgbotcpp
