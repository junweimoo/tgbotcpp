// Minimal example: an echo bot.
//
// This is a skeleton — the handler registration and polling loop are not yet
// implemented. It exists to show the intended shape of the public API and to
// give the build something to compile.

#include <cstdlib>
#include <iostream>

#include "tgbotcpp/tgbotcpp.hpp"

int main() {
    const char* token = std::getenv("TELEGRAM_BOT_TOKEN");
    if (token == nullptr) {
        std::cerr << "Set TELEGRAM_BOT_TOKEN to run this example.\n";
        return 1;
    }

    tgbotcpp::Bot bot(token);

    // bot.onMessage([&](const tgbotcpp::Message& msg) {
    //     bot.api().sendMessage(msg.chat.id, msg.text);
    // });
    // bot.run();

    std::cout << "tgbotcpp echo_bot skeleton.\n";
    return 0;
}
