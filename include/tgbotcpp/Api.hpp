#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "tgbotcpp/net/HttpClient.hpp"
#include "tgbotcpp/types/Message.hpp"
#include "tgbotcpp/types/Update.hpp"

namespace tgbotcpp {

/// Thin, typed wrapper over the Telegram Bot HTTP API
/// (https://core.telegram.org/bots/api). Methods such as getUpdates,
/// sendMessage, etc. are declared here.
///
/// Transport is provided by an injected net::HttpClient; the Api itself is
/// agnostic to how requests actually reach the Telegram servers.
class Api {
public:
    Api(std::string token, net::HttpClient& http);

    const std::string& token() const;

    /// Long-poll for incoming updates. `offset` is the first update_id to
    /// fetch (use last received id + 1 to confirm earlier updates), `timeout`
    /// is the long-poll timeout in seconds, and `limit` caps the batch size.
    std::vector<Update> getUpdates(std::int64_t offset = 0, int timeout = 0, int limit = 100);

    /// Send a text message to the chat identified by `chatId`. Returns the
    /// Message the server reports as sent.
    Message sendMessage(std::int64_t chatId, const std::string& text);

private:
    std::string token_;
    net::HttpClient& http_;
};

} // namespace tgbotcpp
