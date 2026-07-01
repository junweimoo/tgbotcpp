// Plain test runner (no framework yet): each check returns non-zero on failure.
// Replace with a real test framework once one is wired into tests/CMakeLists.txt.

#include <iostream>
#include <string>

#include "net/Url.hpp"
#include "tgbotcpp/tgbotcpp.hpp"

namespace {

// HttpClient fake: returns canned JSON and records the last request so tests can
// assert on what Api sent.
class FakeHttpClient : public tgbotcpp::net::HttpClient {
public:
    std::string getResponse;
    std::string postResponse;

    std::string lastGetUrl;
    std::string lastPostUrl;
    std::string lastPostBody;

    std::string get(const std::string& url) override {
        lastGetUrl = url;
        return getResponse;
    }

    std::string post(const std::string& url, const std::string& body) override {
        lastPostUrl = url;
        lastPostBody = body;
        return postResponse;
    }
};

int failures = 0;

void check(bool condition, const std::string& name) {
    if (!condition) {
        std::cerr << "FAIL: " << name << "\n";
        ++failures;
    }
}

void testTokenAccessor() {
    FakeHttpClient http;
    tgbotcpp::Api api("dummy-token", http);
    check(api.token() == "dummy-token", "token() returns the configured token");
}

void testGetUpdates() {
    FakeHttpClient http;
    http.getResponse = R"({
        "ok": true,
        "result": [
            {
                "update_id": 100,
                "message": {
                    "message_id": 7,
                    "date": 1700000000,
                    "chat": {"id": 42, "type": "private", "username": "alice"},
                    "from": {"id": 42, "is_bot": false, "first_name": "Alice"},
                    "text": "hello"
                }
            }
        ]
    })";

    tgbotcpp::Api api("dummy-token", http);
    auto updates = api.getUpdates(50, 0, 10);

    check(updates.size() == 1, "getUpdates parses one update");
    if (updates.size() == 1) {
        const auto& update = updates.front();
        check(update.updateId == 100, "getUpdates parses update_id");
        check(update.message.messageId == 7, "getUpdates parses message_id");
        check(update.message.text == "hello", "getUpdates parses text");
        check(update.message.chat.id == 42, "getUpdates parses chat.id");
        check(update.message.from.firstName == "Alice", "getUpdates parses from.first_name");
    }
    check(http.lastGetUrl.find("offset=50") != std::string::npos,
          "getUpdates sends offset param");
    check(http.lastGetUrl.find("getUpdates") != std::string::npos,
          "getUpdates calls the getUpdates method");
}

void testSendMessage() {
    FakeHttpClient http;
    http.postResponse = R"({
        "ok": true,
        "result": {
            "message_id": 99,
            "date": 1700000001,
            "chat": {"id": 42, "type": "private"},
            "text": "hi there"
        }
    })";

    tgbotcpp::Api api("dummy-token", http);
    auto message = api.sendMessage(42, "hi there");

    check(message.messageId == 99, "sendMessage parses message_id");
    check(message.text == "hi there", "sendMessage parses text");
    check(message.chat.id == 42, "sendMessage parses chat.id");
    check(http.lastPostUrl.find("sendMessage") != std::string::npos,
          "sendMessage calls the sendMessage method");
    check(http.lastPostBody.find("chat_id=42") != std::string::npos,
          "sendMessage sends chat_id in the body");
    check(http.lastPostBody.find("text=hi%20there") != std::string::npos,
          "sendMessage url-encodes text in the body");
}

void testUpdateKinds() {
    FakeHttpClient http;
    http.getResponse = R"({
        "ok": true,
        "result": [
            {
                "update_id": 200,
                "edited_message": {
                    "message_id": 8,
                    "date": 1700000100,
                    "chat": {"id": 42, "type": "private"},
                    "text": "edited"
                }
            },
            {
                "update_id": 201,
                "callback_query": {
                    "id": "cbq-1",
                    "from": {"id": 7, "is_bot": false, "first_name": "Bob"},
                    "message": {
                        "message_id": 9,
                        "date": 1700000200,
                        "chat": {"id": 42, "type": "private"},
                        "text": "press me"
                    },
                    "chat_instance": "ci-123",
                    "data": "btn:yes"
                }
            },
            {
                "update_id": 202,
                "inline_query": {
                    "id": "iq-1",
                    "from": {"id": 7, "is_bot": false, "first_name": "Bob"},
                    "query": "weather",
                    "offset": "",
                    "chat_type": "private"
                }
            }
        ]
    })";

    tgbotcpp::Api api("dummy-token", http);
    auto updates = api.getUpdates();

    check(updates.size() == 3, "update kinds: parses three updates");
    if (updates.size() == 3) {
        check(updates[0].editedMessage.text == "edited", "parses edited_message");

        const auto& cbq = updates[1].callbackQuery;
        check(cbq.id == "cbq-1", "parses callback_query id");
        check(cbq.from.firstName == "Bob", "parses callback_query from");
        check(cbq.data == "btn:yes", "parses callback_query data");
        check(cbq.message.text == "press me", "parses callback_query message");

        const auto& iq = updates[2].inlineQuery;
        check(iq.id == "iq-1", "parses inline_query id");
        check(iq.query == "weather", "parses inline_query query");
        check(iq.chatType == "private", "parses inline_query chat_type");
    }
}

void testParseUrl() {
    using tgbotcpp::net::parseUrl;

    auto https = parseUrl("https://api.telegram.org/bot123/getUpdates?offset=5");
    check(https.host == "api.telegram.org", "parseUrl host");
    check(https.port == "443", "parseUrl defaults https port to 443");
    check(https.target == "/bot123/getUpdates?offset=5", "parseUrl target keeps path+query");

    auto explicitPort = parseUrl("https://example.com:8443/path");
    check(explicitPort.host == "example.com", "parseUrl host with explicit port");
    check(explicitPort.port == "8443", "parseUrl keeps explicit port");

    auto noPath = parseUrl("https://example.com");
    check(noPath.target == "/", "parseUrl defaults empty path to /");

    auto http = parseUrl("http://example.com/x");
    check(http.port == "80", "parseUrl defaults http port to 80");

    bool threw = false;
    try {
        parseUrl("ftp://example.com");
    } catch (const std::exception&) {
        threw = true;
    }
    check(threw, "parseUrl rejects unsupported scheme");
}

void testHttpClientConstructs() {
    // Verifies OpenSSL links and SSL_CTX initialises; no network is touched.
    bool ok = true;
    try {
        tgbotcpp::net::OpenSslHttpClient client;
        (void)client;
    } catch (const std::exception&) {
        ok = false;
    }
    check(ok, "OpenSslHttpClient constructs without throwing");
}

} // namespace

int main() {
    testTokenAccessor();
    testGetUpdates();
    testSendMessage();
    testUpdateKinds();
    testParseUrl();
    testHttpClientConstructs();

    if (failures == 0) {
        std::cout << "All tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
