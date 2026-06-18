#include "tgbotcpp/Api.hpp"

namespace tgbotcpp {

Api::Api(std::string token)
    : token_(std::move(token)) {}

const std::string& Api::token() const {
    return token_;
}

} // namespace tgbotcpp
