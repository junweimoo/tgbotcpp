# Unit tests.
#
# A test framework (GoogleTest, Catch2, doctest, ...) can be fetched here via
# FetchContent or find_package. For now the suite is a single plain executable
# registered with CTest so `ctest` works out of the box.

add_executable(tgbotcpp_tests test_main.cpp)
target_link_libraries(tgbotcpp_tests PRIVATE tgbotcpp::tgbotcpp)

add_test(NAME tgbotcpp_tests COMMAND tgbotcpp_tests)
