// Plain test runner (no framework yet): each check returns non-zero on failure.
// Replace with a real test framework once one is wired into tests/CMakeLists.txt.

#include <iostream>
#include <string>

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

} // namespace

int main() {
    testTokenAccessor();
    testGetUpdates();
    testSendMessage();

    if (failures == 0) {
        std::cout << "All tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
