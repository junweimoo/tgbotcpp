# tgbotcpp - A Telegram Bot library for C++

tgbotcpp is designed as a high throughput library for integrating with the Telegram API.

## Requirements

- A C++17 compiler (GCC or Clang)
- CMake 3.16 or newer
- OpenSSL development headers (TLS transport for the HTTP client)

On Debian/Ubuntu:

```sh
sudo apt install build-essential cmake libssl-dev
```

## Building

The project uses CMake. A standard out-of-source build:

```sh
cmake -S . -B build
cmake --build build
```

This produces the static library `build/libtgbotcpp.a`, the example
executables, and the test runner.

### Build options

Toggle these with `-D<option>=ON|OFF` at configure time:

| Option                    | Default | Description                       |
| ------------------------- | ------- | --------------------------------- |
| `TGBOTCPP_BUILD_EXAMPLES` | `ON`    | Build the example programs        |
| `TGBOTCPP_BUILD_TESTS`    | `ON`    | Build the unit tests              |
| `TGBOTCPP_BUILD_SHARED`   | `OFF`   | Build as a shared library         |

For example, a release build of just the library:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DTGBOTCPP_BUILD_EXAMPLES=OFF -DTGBOTCPP_BUILD_TESTS=OFF
cmake --build build
```

## Running the tests

```sh
ctest --test-dir build --output-on-failure
```

The suite runs fully offline (HTTP is faked), covering the API
request/response handling and URL parsing.

## Running the example

The `echo_bot` example long-polls Telegram and echoes any text message back to
the chat it came from. It needs a bot token from
[@BotFather](https://t.me/BotFather), passed via the `TELEGRAM_BOT_TOKEN`
environment variable:

```sh
TELEGRAM_BOT_TOKEN=<your-bot-token> ./build/examples/echo_bot
```

Message the bot on Telegram and it will echo your text back. Press Ctrl-C to
stop.

## Quick start

## Receiving messages

## Sending messages

## Handling state
