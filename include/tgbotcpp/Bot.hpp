#pragma once

#include <functional>
#include <string>

#include "tgbotcpp/Api.hpp"
#include "tgbotcpp/types/Update.hpp"

namespace tgbotcpp {

/// Top-level entry point. Owns the API client and the update dispatch loop.
class Bot {
public:
    explicit Bot(std::string token);
    ~Bot();

    /// Access the low-level Telegram Bot API client.
    Api& api();
    const Api& api() const;

    // Update dispatching, handler registration and the long-polling /
    // webhook loop will live here.

private:
    struct Impl;
    // std::unique_ptr<Impl> impl_;  // pimpl, wired up with the implementation
};

} // namespace tgbotcpp
