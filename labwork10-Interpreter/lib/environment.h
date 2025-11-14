#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>

class Value;

class Environment {
    std::unordered_map<std::string, Value> values_;
    std::shared_ptr<Environment> parent_;
public:
    Environment();
    Environment(std::shared_ptr<Environment> parent);

    static std::shared_ptr<Environment> create_global();
    static std::shared_ptr<Environment> create_child(std::shared_ptr<Environment> parent);
    void declare(const std::string& name, const Value& value);
    void assign(const std::string& name, const Value& value);
    Value get(const std::string& name) const;
};