# Each example is a standalone executable linked against the library.

add_executable(echo_bot echo_bot.cpp)
target_link_libraries(echo_bot PRIVATE tgbotcpp::tgbotcpp)
