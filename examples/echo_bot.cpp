// Minimal example: an echo bot.
//
// Long-polls Telegram for updates and echoes any text message back to the chat
// it came from. Bot has no handler/run loop yet, so this drives bot.api()
// directly.

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>

#include "tgbotcpp/tgbotcpp.hpp"

int main() {
    const char* token = std::getenv("TELEGRAM_BOT_TOKEN");
    if (token == nullptr) {
        std::cerr << "Set TELEGRAM_BOT_TOKEN to run this example.\n";
        return 1;
    }

    tgbotcpp::Bot bot(token);
    std::cout << "tgbotcpp echo_bot running. Press Ctrl-C to stop.\n";

    std::int64_t offset = 0;
    try {
        while (true) {
            for (const auto& update : bot.api().getUpdates(offset, 30)) {
                offset = update.updateId + 1;
                if (!update.message.text.empty()) {
                    bot.api().sendMessage(update.message.chat.id, update.message.text);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "echo_bot error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
