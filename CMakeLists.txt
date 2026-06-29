cmake_minimum_required(VERSION 3.16)

project(tgbotcpp
    VERSION 0.1.0
    DESCRIPTION "A high throughput Telegram Bot library for C++"
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(TGBOTCPP_BUILD_EXAMPLES "Build the example programs" ON)
option(TGBOTCPP_BUILD_TESTS    "Build the unit tests"       ON)
option(TGBOTCPP_BUILD_SHARED   "Build as a shared library"  OFF)

# Honour the BUILD_SHARED_LIBS convention as well.
if(TGBOTCPP_BUILD_SHARED)
    set(BUILD_SHARED_LIBS ON)
endif()

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(GNUInstallDirs)

# ---------------------------------------------------------------------------
# Library target
# ---------------------------------------------------------------------------
file(GLOB_RECURSE TGBOTCPP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(tgbotcpp ${TGBOTCPP_SOURCES})
add_library(tgbotcpp::tgbotcpp ALIAS tgbotcpp)

target_include_directories(tgbotcpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(tgbotcpp PUBLIC cxx_std_17)

set_target_properties(tgbotcpp PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Dependencies.
# OpenSSL provides the TLS transport for the HTTP client. It is linked PRIVATE
# because OpenSSL types stay behind a pimpl and never appear in public headers;
# CMake still propagates it to consumers of this static library at link time.
find_package(OpenSSL REQUIRED)
target_link_libraries(tgbotcpp PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# ---------------------------------------------------------------------------
# Subdirectories
# ---------------------------------------------------------------------------
if(TGBOTCPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(TGBOTCPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
install(TARGETS tgbotcpp
    EXPORT tgbotcppTargets
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
