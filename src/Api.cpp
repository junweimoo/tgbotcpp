#include "tgbotcpp/Api.hpp"

#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "json/Json.hpp"

namespace tgbotcpp {
namespace {

// ---------------------------------------------------------------------------
// URL / form helpers
// ---------------------------------------------------------------------------

std::string apiUrl(const std::string& token, const std::string& method) {
    return "https://api.telegram.org/bot" + token + "/" + method;
}

std::string urlEncode(const std::string& value) {
    static const char* kHex = "0123456789ABCDEF";
    std::string out;
    out.reserve(value.size());
    for (unsigned char c : value) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out.push_back(static_cast<char>(c));
        } else {
            out.push_back('%');
            out.push_back(kHex[c >> 4]);
            out.push_back(kHex[c & 0x0F]);
        }
    }
    return out;
}

// Incrementally assembles a `key=value&key=value` form/query string.
class FormBuilder {
public:
    void add(const std::string& key, const std::string& value) {
        if (!body_.empty()) {
            body_.push_back('&');
        }
        body_ += urlEncode(key);
        body_.push_back('=');
        body_ += urlEncode(value);
    }

    void add(const std::string& key, std::int64_t value) {
        add(key, std::to_string(value));
    }

    const std::string& str() const { return body_; }
    bool empty() const { return body_.empty(); }

private:
    std::string body_;
};

// ---------------------------------------------------------------------------
// JSON -> struct mapping
// ---------------------------------------------------------------------------

using json::JsonValue;

User parseUser(const JsonValue& j) {
    User user;
    user.id = j.at("id").asInt64();
    user.isBot = j.at("is_bot").asBool();
    user.firstName = j.at("first_name").asString();
    user.lastName = j.at("last_name").asString();
    user.username = j.at("username").asString();
    return user;
}

Chat parseChat(const JsonValue& j) {
    Chat chat;
    chat.id = j.at("id").asInt64();
    chat.type = j.at("type").asString();
    chat.title = j.at("title").asString();
    chat.username = j.at("username").asString();
    return chat;
}

Message parseMessage(const JsonValue& j) {
    Message message;
    message.messageId = j.at("message_id").asInt64();
    message.date = j.at("date").asInt64();
    if (j.has("chat")) {
        message.chat = parseChat(j.at("chat"));
    }
    if (j.has("from")) {
        message.from = parseUser(j.at("from"));
    }
    message.text = j.at("text").asString();
    return message;
}

Update parseUpdate(const JsonValue& j) {
    Update update;
    update.updateId = j.at("update_id").asInt64();
    if (j.has("message")) {
        update.message = parseMessage(j.at("message"));
    }
    return update;
}

JsonValue unwrapResult(const std::string& response) {
    JsonValue root = json::parse(response);
    if (!root.isObject()) {
        throw std::runtime_error("Telegram API: response is not a JSON object");
    }
    if (!root.at("ok").asBool()) {
        std::string description = root.at("description").asString();
        throw std::runtime_error("Telegram API error: " +
                                 (description.empty() ? "ok=false" : description));
    }
    return root.at("result");
}

} // namespace

Api::Api(std::string token, net::HttpClient& http)
    : token_(std::move(token)), http_(http) {}

const std::string& Api::token() const {
    return token_;
}

std::vector<Update> Api::getUpdates(std::int64_t offset, int timeout, int limit) {
    FormBuilder form;
    if (offset != 0) {
        form.add("offset", offset);
    }
    form.add("timeout", static_cast<std::int64_t>(timeout));
    form.add("limit", static_cast<std::int64_t>(limit));

    std::string url = apiUrl(token_, "getUpdates") + "?" + form.str();
    JsonValue result = unwrapResult(http_.get(url));

    std::vector<Update> updates;
    if (result.isArray()) {
        updates.reserve(result.items().size());
        for (const JsonValue& item : result.items()) {
            updates.push_back(parseUpdate(item));
        }
    }
    return updates;
}

Message Api::sendMessage(std::int64_t chatId, const std::string& text) {
    FormBuilder form;
    form.add("chat_id", chatId);
    form.add("text", text);

    std::string url = apiUrl(token_, "sendMessage");
    JsonValue result = unwrapResult(http_.post(url, form.str()));
    return parseMessage(result);
}

} // namespace tgbotcpp
