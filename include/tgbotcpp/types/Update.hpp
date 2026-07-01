#pragma once

#include <cstdint>

#include "tgbotcpp/types/CallbackQuery.hpp"
#include "tgbotcpp/types/InlineQuery.hpp"
#include "tgbotcpp/types/Message.hpp"

namespace tgbotcpp {

/// Mirrors the Telegram `Update` object — a single incoming event. Exactly one
/// of the payload fields is populated per update; the rest stay default.
struct Update {
    std::int64_t updateId = 0;

    // Message-typed update kinds (reuse the Message type).
    Message message;
    Message editedMessage;
    Message channelPost;
    Message editedChannelPost;
    Message businessMessage;
    Message editedBusinessMessage;
    Message guestMessage;

    // Query update kinds.
    CallbackQuery callbackQuery;
    InlineQuery inlineQuery;

    // Other kinds (business_connection, poll, chat_member, ...) go here.
};

} // namespace tgbotcpp
