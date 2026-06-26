#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace tgbotcpp::json {

class JsonValue {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    JsonValue() : type_(Type::Null) {}

    Type type() const { return type_; }
    bool isObject() const { return type_ == Type::Object; }
    bool isArray() const { return type_ == Type::Array; }

    bool asBool() const { return bool_; }
    const std::string& asString() const { return string_; }

    std::int64_t asInt64() const {
        return string_.empty() ? 0 : static_cast<std::int64_t>(std::stoll(string_));
    }

    const std::vector<JsonValue>& items() const { return array_; }

    const JsonValue& at(const std::string& key) const {
        static const JsonValue kNull;
        auto it = object_.find(key);
        return it == object_.end() ? kNull : it->second;
    }

    bool has(const std::string& key) const { return object_.count(key) > 0; }

    // -- mutators used by the parser --
    void setBool(bool v) { type_ = Type::Bool; bool_ = v; }
    void setNumber(std::string v) { type_ = Type::Number; string_ = std::move(v); }
    void setString(std::string v) { type_ = Type::String; string_ = std::move(v); }
    void setArray(std::vector<JsonValue> v) { type_ = Type::Array; array_ = std::move(v); }
    void setObject(std::map<std::string, JsonValue> v) {
        type_ = Type::Object;
        object_ = std::move(v);
    }

private:
    Type type_;
    bool bool_ = false;
    std::string string_;
    std::vector<JsonValue> array_;
    std::map<std::string, JsonValue> object_;
};

JsonValue parse(const std::string& text);

} // namespace tgbotcpp::json
