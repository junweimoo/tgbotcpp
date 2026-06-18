#pragma once

#include <cstdint>

#include "tgbotcpp/types/Message.hpp"

namespace tgbotcpp {

/// Mirrors the Telegram `Update` object — a single incoming event.
struct Update {
    std::int64_t updateId = 0;
    Message message;
    // Other update kinds (edited_message, callback_query, ...) go here.
};

} // namespace tgbotcpp
