# Unit tests.
#
# A test framework (GoogleTest, Catch2, doctest, ...) can be fetched here via
# FetchContent or find_package. For now the suite is a single plain executable
# registered with CTest so `ctest` works out of the box.

add_executable(tgbotcpp_tests test_main.cpp)
target_link_libraries(tgbotcpp_tests PRIVATE tgbotcpp::tgbotcpp)

# Allow the tests to reach internal headers (e.g. net/Url.hpp) that are not part
# of the public include interface.
target_include_directories(tgbotcpp_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_test(NAME tgbotcpp_tests COMMAND tgbotcpp_tests)
