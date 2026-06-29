#pragma once

#include <functional>
#include <memory>
#include <string>

#include "tgbotcpp/Api.hpp"
#include "tgbotcpp/types/Update.hpp"

namespace tgbotcpp {

class Bot {
public:
    explicit Bot(std::string token);
    ~Bot();

    Api& api();
    const Api& api() const;

    // Update dispatching, handler registration and the long-polling /
    // webhook loop will live here.

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace tgbotcpp
