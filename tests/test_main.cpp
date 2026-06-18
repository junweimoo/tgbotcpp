// Placeholder test runner. Replace the body with real assertions once a test
// framework is wired into tests/CMakeLists.txt.

#include "tgbotcpp/tgbotcpp.hpp"

int main() {
    tgbotcpp::Api api("dummy-token");
    return api.token() == "dummy-token" ? 0 : 1;
}
