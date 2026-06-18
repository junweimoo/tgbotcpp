#pragma once

#include <string>

namespace tgbotcpp {

/// Thin, typed wrapper over the Telegram Bot HTTP API
/// (https://core.telegram.org/bots/api). Methods such as getUpdates,
/// sendMessage, etc. are declared here.
class Api {
public:
    explicit Api(std::string token);

    const std::string& token() const;

private:
    std::string token_;
};

} // namespace tgbotcpp
