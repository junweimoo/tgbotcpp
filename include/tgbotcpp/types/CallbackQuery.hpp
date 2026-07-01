#pragma once

#include <string>

#include "tgbotcpp/types/Message.hpp"
#include "tgbotcpp/types/User.hpp"

namespace tgbotcpp {

/// Mirrors the Telegram `CallbackQuery` object — an incoming callback from an
/// inline keyboard button.
struct CallbackQuery {
    std::string id;
    User from;
    Message message;  // message bearing the callback button (if any)
    std::string inlineMessageId;
    std::string chatInstance;
    std::string data;
    std::string gameShortName;
};

} // namespace tgbotcpp
