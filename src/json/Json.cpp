#include "json/Json.hpp"

#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace tgbotcpp::json {
namespace {

class JsonParser {
public:
    explicit JsonParser(const std::string& text) : text_(text) {}

    JsonValue parse() {
        JsonValue value = parseValue();
        skipWhitespace();
        if (pos_ != text_.size()) {
            throw std::runtime_error("JSON: trailing characters after value");
        }
        return value;
    }

private:
    const std::string& text_;
    std::size_t pos_ = 0;

    void skipWhitespace() {
        while (pos_ < text_.size() &&
               (text_[pos_] == ' ' || text_[pos_] == '\t' || text_[pos_] == '\n' ||
                text_[pos_] == '\r')) {
            ++pos_;
        }
    }

    char peek() {
        if (pos_ >= text_.size()) {
            throw std::runtime_error("JSON: unexpected end of input");
        }
        return text_[pos_];
    }

    void expect(char c) {
        if (peek() != c) {
            throw std::runtime_error(std::string("JSON: expected '") + c + "'");
        }
        ++pos_;
    }

    JsonValue parseValue() {
        skipWhitespace();
        switch (peek()) {
            case '{': return parseObject();
            case '[': return parseArray();
            case '"': {
                JsonValue v;
                v.setString(parseString());
                return v;
            }
            case 't':
            case 'f': return parseBool();
            case 'n': return parseNull();
            default: return parseNumber();
        }
    }

    JsonValue parseObject() {
        expect('{');
        std::map<std::string, JsonValue> members;
        skipWhitespace();
        if (peek() == '}') {
            ++pos_;
        } else {
            while (true) {
                skipWhitespace();
                std::string key = parseString();
                skipWhitespace();
                expect(':');
                members.emplace(std::move(key), parseValue());
                skipWhitespace();
                char c = peek();
                ++pos_;
                if (c == '}') break;
                if (c != ',') throw std::runtime_error("JSON: expected ',' or '}'");
            }
        }
        JsonValue v;
        v.setObject(std::move(members));
        return v;
    }

    JsonValue parseArray() {
        expect('[');
        std::vector<JsonValue> items;
        skipWhitespace();
        if (peek() == ']') {
            ++pos_;
        } else {
            while (true) {
                items.push_back(parseValue());
                skipWhitespace();
                char c = peek();
                ++pos_;
                if (c == ']') break;
                if (c != ',') throw std::runtime_error("JSON: expected ',' or ']'");
            }
        }
        JsonValue v;
        v.setArray(std::move(items));
        return v;
    }

    std::string parseString() {
        expect('"');
        std::string out;
        while (true) {
            if (pos_ >= text_.size()) {
                throw std::runtime_error("JSON: unterminated string");
            }
            char c = text_[pos_++];
            if (c == '"') break;
            if (c == '\\') {
                char esc = text_.at(pos_++);
                switch (esc) {
                    case '"': out.push_back('"'); break;
                    case '\\': out.push_back('\\'); break;
                    case '/': out.push_back('/'); break;
                    case 'b': out.push_back('\b'); break;
                    case 'f': out.push_back('\f'); break;
                    case 'n': out.push_back('\n'); break;
                    case 'r': out.push_back('\r'); break;
                    case 't': out.push_back('\t'); break;
                    case 'u': out += decodeUnicodeEscape(); break;
                    default: throw std::runtime_error("JSON: invalid escape");
                }
            } else {
                out.push_back(c);
            }
        }
        return out;
    }

    // Decodes a single \uXXXX escape (already consumed the 'u') into UTF-8.
    // Surrogate pairs are not handled; sufficient for the basic types here.
    std::string decodeUnicodeEscape() {
        if (pos_ + 4 > text_.size()) {
            throw std::runtime_error("JSON: truncated \\u escape");
        }
        unsigned int code = 0;
        for (int i = 0; i < 4; ++i) {
            char c = text_[pos_++];
            code <<= 4;
            if (c >= '0' && c <= '9') code |= static_cast<unsigned>(c - '0');
            else if (c >= 'a' && c <= 'f') code |= static_cast<unsigned>(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') code |= static_cast<unsigned>(c - 'A' + 10);
            else throw std::runtime_error("JSON: invalid \\u escape");
        }
        std::string out;
        if (code < 0x80) {
            out.push_back(static_cast<char>(code));
        } else if (code < 0x800) {
            out.push_back(static_cast<char>(0xC0 | (code >> 6)));
            out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xE0 | (code >> 12)));
            out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
        }
        return out;
    }

    JsonValue parseNumber() {
        std::size_t start = pos_;
        while (pos_ < text_.size()) {
            char c = text_[pos_];
            if ((c >= '0' && c <= '9') || c == '-' || c == '+' || c == '.' || c == 'e' ||
                c == 'E') {
                ++pos_;
            } else {
                break;
            }
        }
        if (pos_ == start) {
            throw std::runtime_error("JSON: invalid value");
        }
        JsonValue v;
        v.setNumber(text_.substr(start, pos_ - start));
        return v;
    }

    JsonValue parseBool() {
        JsonValue v;
        if (text_.compare(pos_, 4, "true") == 0) {
            pos_ += 4;
            v.setBool(true);
        } else if (text_.compare(pos_, 5, "false") == 0) {
            pos_ += 5;
            v.setBool(false);
        } else {
            throw std::runtime_error("JSON: invalid literal");
        }
        return v;
    }

    JsonValue parseNull() {
        if (text_.compare(pos_, 4, "null") != 0) {
            throw std::runtime_error("JSON: invalid literal");
        }
        pos_ += 4;
        return JsonValue();
    }
};

} // namespace

JsonValue parse(const std::string& text) {
    return JsonParser(text).parse();
}

} // namespace tgbotcpp::json
